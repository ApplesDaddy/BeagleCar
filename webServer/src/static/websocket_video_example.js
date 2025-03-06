// https://developer.mozilla.org/en-US/docs/Web/API/SourceBuffer


$(document).ready(function () {
    const video = document.getElementById('video');
    const mediaSource = new MediaSource();
    video.src = URL.createObjectURL(mediaSource);
  
    mediaSource.addEventListener('sourceopen', () => {
      const ws = new WebSocket('ws://192.168.7.2:8080/ws_video');
      const sourceBuffer = mediaSource.addSourceBuffer('video/mp4; codecs="avc1.64001F"');
  
      ws.binaryType = 'arraybuffer';
      ws.onmessage = (event) => {
        sourceBuffer.appendBuffer(event.data); // Append received MP4 fragments
      };
    });
});

