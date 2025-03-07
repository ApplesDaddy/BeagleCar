// https://developer.mozilla.org/en-US/docs/Web/API/SourceBuffer



const video = document.getElementById('video');
const mediaSource = new MediaSource();
video.src = URL.createObjectURL(mediaSource);

var event_item;


//TODO: add an event listner for updateend to stop the 
// never ending loading
// source: https://stackoverflow.com/questions/79182435/video-using-mediasource-api-showing-unending-loading-screen

mediaSource.addEventListener('sourceopen', () => {
  const ws = new WebSocket('ws://localhost:8080/ws_video');
  const sourceBuffer = mediaSource.addSourceBuffer('video/mp4');
  let bufferQueue = []; // Queue for incoming video data
  let isAppending = false; // Track appending state

  ws.binaryType = 'arraybuffer';

  ws.onmessage = (event) => {
    console.log("Received a file. Object to follow:");
    console.log(event);
    event_item = event;

    bufferQueue.push(event.data); // Store data in queue
    appendNextBuffer(); // Attempt to append data
  };

  function appendNextBuffer() {
    if (!isAppending && bufferQueue.length > 0 && !sourceBuffer.updating) {
      isAppending = true; // Mark appending state
      sourceBuffer.appendBuffer(bufferQueue.shift()); // Append next chunk
    }
  }

  sourceBuffer.addEventListener('updateend', () => {
    isAppending = false; // Reset flag
    appendNextBuffer(); // Try appending next chunk if available
  });

  sourceBuffer.addEventListener('error', (e) => {
    console.error("SourceBuffer error:", e);
  });
});

