
var gl;

window.onload = function init()
{
    //Get Canvas
    var canvas = document.getElementById("gl-canvas");

    //Get WebGL Context
    gl = WebGLUtils.setupWebGL(canvas);
    if (!gl)
    {
        alert("Failed to get context!");
        return;
    }

    //Setup viewport and clear color
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.3921, 0.5843, 0.9294, 1.0);

    //Draw stuff
    render();
}

function render()
{
    gl.clear(gl.COLOR_BUFFER_BIT);
}