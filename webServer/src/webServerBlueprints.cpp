#include "crow_all.h"
#include "webServer/webServerBlueprints.h"
#include <stdlib.h>
#include <sys/wait.h>

void close_port();
/*
References
    https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/2_remuxing.c
    https://developer.mozilla.org/en-US/docs/Web/API/Media_Source_Extensions_API/Transcoding_assets_for_MSE
    https://ffmpeg.org/ffmpeg-protocols.html#udp
    https://stackoverflow.com/questions/52303867/how-do-i-set-ffmpeg-pipe-output
    https://stackoverflow.com/questions/52396602/conversion-from-mjpeg-to-mp4-libx264-with-ffmpeg
*/
#define FFMPEG_CMD "ffmpeg"
#define FFLAGS "-fflags"
#define FFLAGS_ARG  "+genpts+igndts+discardcorrupt"
#define INPUT "-i"
#define IP_ADDR "192.168.7.2"
// Warning when trying to set buffer_size to ~8mb bc mjpeg packets are large; limited by OS
#define INPUT_URL_ARGS "?buffer_size=7713000fifo_size=5000000&overrun_nonfatal=1"
#define INPUT_URL "udp://" IP_ADDR ":" PORT INPUT_URL_ARGS
#define COPY "-c:v"
#define COPY_ARG "libx264"
#define MOVFLAG "-movflags"
#define MOVFLAG_ARGS "frag_keyframe+empty_moov+default_base_moof"
#define FORMAT "-f"
#define FORMAT_ARG "mp4"
#define OUTPUT "pipe:1"
#define PIX_FMT "-pix_fmt"
#define PIX_FMT_ARG "yuv420p"
#define BITRATE "-b:v"
#define BITRATE_ARG "4000k"
#define TUNE "-tune"
#define TUNE_ARG "zerolatency"
#define PRESET "-preset"
#define PRESET_ARG "ultrafast"

int p_id, pipe_fd[2];

// Example from: https://crowcpp.org/master/getting_started/your_first_application/
void add_routes(crow::SimpleApp& app){
    
    //define your endpoint at the root directory
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });


    //Return a template
    CROW_ROUTE(app, "/template/<int>")([](int count){
        auto page = crow::mustache::compile("The value is {{value}}");
        crow::mustache::context ctx;
        ctx["value"] = count;

        return page.render(ctx);
    });


    //Return a template from a file with values
    CROW_ROUTE(app, "/template_file/<int>")([](int count){
        auto page = crow::mustache::load("template_page.html");
        crow::mustache::context ctx;
        ctx["value"] = count;

        return page.render(ctx);
    });

    //Return a template from a file
    CROW_ROUTE(app, "/load_file")([](){
        auto page = crow::mustache::load_text("template_page.html");
        
        return page;
    });

    // From: https://crowcpp.org/master/guides/websockets/
    // Only prints out to the console for now. To get a 
    // connection go to /static/websocket_page.html
    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn){
            CROW_LOG_INFO << "new websocket connection";
            CROW_LOG_INFO << "ip address of new remote connection: " <<  conn.get_remote_ip();
            })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary){
                if (is_binary){
                    CROW_LOG_INFO << "received binary message: " << data;
                    conn.send_binary(data);
                } else {
                    CROW_LOG_INFO << "received message: " << data;
                    conn.send_text(data);
                }
            });

    CROW_WEBSOCKET_ROUTE(app, "/ws_video")
    .onopen([&](crow::websocket::connection& conn){
        CROW_LOG_INFO << "new websocket connection";
        CROW_LOG_INFO << "ip address: " << conn.get_remote_ip();

    
        int status; 

        // Credit: https://www2.cs.uregina.ca/~hamilton/courses/330/notes/unix/pipes/pipe.cpp 
        status = pipe(pipe_fd); // pipe_fd := file descriptors 
        if(status == -1){
            std::cout << "Error: Could not create pipe\n";
            exit(1);
        }

        p_id = fork();
        if(p_id == -1){
            std::cout << "Error: Could not fork process\n";
            exit(1);
        } else if (p_id == 0) { // Child process: Write to pipe
            close(pipe_fd[0]); // Close read end

            /*
            References: 
                https://en.wikipedia.org/wiki/Redirection_(computing)
                https://en.wikipedia.org/wiki/Dup_(system_call)
                https://stackoverflow.com/questions/22367920/is-it-possible-that-linux-file-descriptor-0-1-2-not-for-stdin-stdout-and-stderr
                https://en.wikipedia.org/wiki/File_descriptor
                    
            */
            // redirect output from stdout to pipe
            dup2(pipe_fd[1], STDOUT_FILENO);

            // Write to pipe
            // https://stackoverflow.com/questions/39873304/include-ffmpeg-command-in-c-program
            // gave up trying to write remuxer using libav; ffmpeg system call instead
            // replaces the current process image with ffmpeg's process image
            if(execlp(FFMPEG_CMD, FFMPEG_CMD, FFLAGS, FFLAGS_ARG, INPUT, INPUT_URL, PIX_FMT, 
                PIX_FMT_ARG, BITRATE, BITRATE_ARG, COPY, COPY_ARG, PRESET, PRESET_ARG,
                TUNE, TUNE_ARG, MOVFLAG, MOVFLAG_ARGS, FORMAT, FORMAT_ARG, OUTPUT, (const char *) NULL) == -1){
                std::cout << "Error: Launching ffmpeg failed\n";
            }

            close(pipe_fd[1]); // Close write end after writing
            exit(0);
        }
        std::thread t(send_video_websocket, std::ref(conn), pipe_fd[0]);
        t.detach();
    })

    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            // pid_t p_id = fork();
            // if(p_id == -1){
            //     std::cout << "Error forking on close\n";
            // } else if (p_id == 0){
            //     close_port();
            // }
            if(p_id > 0){
                kill(0, SIGTERM);
                waitpid(0, nullptr, 0);
            }
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            conn.close();
            });

}

void send_video_websocket(crow::websocket::connection& conn, int pipe_fd){
            // Read from pipe and send to websocket
            char data[1024 * 1024]; // 1Mb buffer size
            size_t read_size;
            while ((read_size = read(pipe_fd, data, sizeof(data))) > 0) {
                conn.send_binary(std::string(data, read_size));  // Send binary data as a string
                // read_size = 0;
                // memset(data, 0, sizeof(data));
            }
            // Close read end
            close(pipe_fd);
}

void send_video_websocket_sample(crow::websocket::connection& conn){
    for (int i = 0; i < 16; i++){
        char file_name[100];
        sprintf(file_name, "output%03d.mp4", i);

        CROW_LOG_INFO << "Sending: " << file_name;

        FILE* file = fopen(file_name, "rb");
        if (file == NULL){
            CROW_LOG_INFO << "Could not open file: " << file_name;
            return;
        }

        char data[600 * 1024]; // 400 KB - since the largest 2 seocond file seems to be about 300 KB
        size_t read_size = fread(data, 1, sizeof(data), file);
        if (read_size == 0){
            CROW_LOG_INFO << "Could not read file: " << file_name;
            return;
        }

        conn.send_binary(std::string(data, read_size));
        fclose(file);

        std::this_thread::sleep_for(std::chrono::milliseconds(2100));
    }
    conn.close();
}

// Credit: https://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event
// credit: https://stackoverflow.com/questions/11583562/how-to-kill-a-process-running-on-particular-port-in-linux
// Close ffmpeg port on interrupt
void signal_handler(int i)
{
    std::cout << "Caught signal " << i << std::endl;
    close_port();
    exit(1);
}

void close_port()
{
    if(execlp(FUSER_CMD, FUSER_CMD, CLOSE_PORT_CMD, (const char *) NULL) == -1){
        std::cout << "Error: Closing udp port failed\n";
    }
}