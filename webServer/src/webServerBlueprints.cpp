#include "crow_all.h"
#include "webServer/webServerBlueprints.h"
#include <stdlib.h>
#include <sys/wait.h>

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

    int status, p_id, pipe_fd[2]; 

    CROW_WEBSOCKET_ROUTE(app, "/ws_video")
    .onopen([&](crow::websocket::connection& conn){
        CROW_LOG_INFO << "new websocket connection";
        CROW_LOG_INFO << "ip address: " << conn.get_remote_ip();

        /*
        References
            https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/2_remuxing.c
            https://developer.mozilla.org/en-US/docs/Web/API/Media_Source_Extensions_API/Transcoding_assets_for_MSE
            https://ffmpeg.org/ffmpeg-protocols.html#udp
            https://stackoverflow.com/questions/52303867/how-do-i-set-ffmpeg-pipe-output
        */
        std::string fflags = "-fflags +genpts+igndts+discardcorrupt";
        std::string input_url = "-i \"udp://192.168.7.2:12345";
        std::string input_url_opts = "?fifo_size=5000000&overrun_nonfatal=1\"";
        std::string copy_flag = "-c:v copy";
        std::string mov_flags = "-movflags frag_keyframe+empty_moov+default_base_moof";
        std::string format_flag = "-f mp4";
        std::string output_file = "pipe:1";
        std::string ffmpeg_cmd = "ffmpeg " + fflags + " " + input_url + " " + input_url_opts + " " 
        + copy_flag + " " + mov_flags + " " + format_flag + " " + output_file;
    

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
            const char* ffmpeg_cmd_c = ffmpeg_cmd.c_str();
            // replaces the current process image with ffmpeg's process image
            if(execl("/bin/sh", "sh", "-c", ffmpeg_cmd_c, (char *) NULL) == errno){
                std::cout << "Error: Launching ffmpeg failed\n";
            }

            close(pipe_fd[1]); // Close write end after writing
            exit(0);
        }
        std::thread t(send_video_websocket, std::ref(conn), pipe_fd[0]);
        t.detach();
    })

    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            // credit: https://stackoverflow.com/questions/11583562/how-to-kill-a-process-running-on-particular-port-in-linux
            system("fuser -k 12345/udp"); // Close ffmpeg udp connection
            close(pipe_fd[1]);
            close(pipe_fd[0]);
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            conn.close();
            });

}

void send_video_websocket(crow::websocket::connection& conn, int pipe_fd){
            // Read from pipe and send to websocket
            char data[600 * 1024]; // TODO: increase size bc higher resolution
            size_t read_size;
            while ((read_size = read(pipe_fd, data, sizeof(data))) > 0) {
                conn.send_binary(std::string(data, read_size));  // Send binary data as a string
                read_size = 0;
                memset(data, 0, sizeof(data));
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