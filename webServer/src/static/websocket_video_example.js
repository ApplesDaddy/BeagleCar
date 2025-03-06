// https://developer.mozilla.org/en-US/docs/Web/API/SourceBuffer



const video = document.getElementById('video');
const mediaSource = new MediaSource();
video.src = URL.createObjectURL(mediaSource);

//TODO: add an event listner for updateend to stop the 
// never ending loading
// source: https://stackoverflow.com/questions/79182435/video-using-mediasource-api-showing-unending-loading-screen

mediaSource.addEventListener('sourceopen', () => {
  const ws = new WebSocket('ws://localhost:8080/ws_video');
  const sourceBuffer = mediaSource.addSourceBuffer('video/mp4');

  ws.binaryType = 'arraybuffer';
  ws.onmessage = (event) => {
    console.log("Received a file. Object to follow:")
    console.log(event)
    sourceBuffer.appendBuffer(event.data); // Append received MP4 fragments
  };
});

