"use strict";

var gl;

var vertexBuffer;
var vertexElementArrayBuffer;
var shaderProgram;

var modelData = boxData;
var LightPosition = [0, 0, -1, 1];

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
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);


    //Load, compile and link the shaders
    shaderProgram = initShaders(gl, "vertex-shader", "fragment-shader");
    gl.useProgram(shaderProgram);

    //load interleaved vertex buffer
    createVertexBufferObject(modelData[0]);

    //setup keyboard event handler
    initKeyboard();

    //load the texture and setup the OpenGL texture
    loadTexture();

    //start draw loop
    render();
}

function render()
{
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var fovy = 60;
    var aspect = 1;
    var near = 0.1;
    var far = 100;
    var theta = Date.now() / 100;


    var eye = vec3(0, 0, 3);
    var at = vec3(0, 0, 0);
    var up = vec3(0, 1, 0);

    //****************************************************
    //* TODO: Create projection, view and model matrices
    //****************************************************
    var projection = perspective(fovy, aspect, near, far);
    var view = lookAt(eye, at, up);
    var model = rotate(theta, vec3(0, 1, 1));
    var modelView = mult(view, model);

    //****************************************************
    //* TODO: Upload the modelView and projection
    //*       matrix to the GPU
    //****************************************************
    gl.uniformMatrix4fv(gl.getUniformLocation(shaderProgram, "ModelView"), false, flatten(modelView));
    gl.uniformMatrix4fv(gl.getUniformLocation(shaderProgram, "Projection"), false, flatten(projection));

    //upload light position vector and bind 'texture' to sampler 0
    gl.uniform4fv(gl.getUniformLocation(shaderProgram, "LightPosition"), LightPosition);
    gl.uniform1i(gl.getUniformLocation(shaderProgram, "texture"), 0);
    
    //bind vertex buffer and associate it with shader attributes
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    bindAttributes();

    //bind index buffer
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vertexElementArrayBuffer);

    //draw using vertex and index buffer
    gl.drawElements(gl.TRIANGLES, modelData[0].indices0.length, gl.UNSIGNED_SHORT, 0);

    //request the render method to be called again when "needed"
    //this is similar to the display callback when using GLUT
    requestAnimFrame(render);
}

function bindAttributes()
{
    //*************************************************
    //* TODO: Extend the vertex declaration to include
    //*       the attribute 'uv' (texture coordinates)
    //*************************************************

    var sizeOfFloat = 4;
    var vertexLength = (3 + 3 + 2) * sizeOfFloat;

    var vertexLocation = gl.getAttribLocation(shaderProgram, "position");
    gl.enableVertexAttribArray(vertexLocation);
    gl.vertexAttribPointer(vertexLocation, 3, gl.FLOAT, false, vertexLength, 0);

    var normalLocation = gl.getAttribLocation(shaderProgram, "normal");
    gl.enableVertexAttribArray(normalLocation);
    gl.vertexAttribPointer(normalLocation, 3, gl.FLOAT, false, vertexLength, 3 * sizeOfFloat);

    var uvLocation = gl.getAttribLocation(shaderProgram, "uv");
    gl.enableVertexAttribArray(uvLocation);
    gl.vertexAttribPointer(uvLocation, 2, gl.FLOAT, false, vertexLength, 6 * sizeOfFloat);
}

function createVertexBufferObject(data)
{
    //*************************************************
    //* TODO: Extend the interleaved data loading to
    //*       also load the texture coordinates from
    //*       the mesh data (data.uv1).
    //*************************************************


    // create interleaved data
    var vertexCount = data.vertex.length / 3;
    var vertexSize = 3 + 3 + 2;
    var vertices = new Float32Array(vertexCount * vertexSize);

    for (var i = 0; i < vertexCount; i++)
    {
        vertices[i * vertexSize] = data.vertex[i * 3];
        vertices[i * vertexSize + 1] = data.vertex[i * 3 + 1];
        vertices[i * vertexSize + 2] = data.vertex[i * 3 + 2];
        vertices[i * vertexSize + 3] = data.normal[i * 3];
        vertices[i * vertexSize + 4] = data.normal[i * 3 + 1];
        vertices[i * vertexSize + 5] = data.normal[i * 3 + 2];
        vertices[i * vertexSize + 6] = data.uv1[i * 2];
        vertices[i * vertexSize + 7] = data.uv1[i * 2 + 1];
    }

    //create and bind vertex buffer and upload vertices data
    vertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    //create and bind index buffer and upload indices data
    vertexElementArrayBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vertexElementArrayBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(data.indices0), gl.STATIC_DRAW);
}

function initKeyboard()
{
    document.onkeyup = function(event)
    {
        if (String.fromCharCode(event.keyCode) == 'L')
        {
            LightPosition[3] = (LightPosition[3] + 1) % 2; //toggle LightPosition.w = 0 or 1
            console.log(LightPosition[3] == 0 ? "Directional light" : "Point light"); //press F12 in IE and Chrome, Shift+F2 in Firefox to bring up the console
        }
    };
}

function loadTexture()
{
    //create an image object
    var image = new Image();

    //create an onload event handler for the image
    image.onload = function ()
    {
        var texID = gl.createTexture();
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texID);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    };

    //set image data source, this will invoke the onload event
    image.src = imageData;
}