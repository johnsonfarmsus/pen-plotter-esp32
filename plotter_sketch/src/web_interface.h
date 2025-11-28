#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// Simplified HTML interface embedded as a string
// Full-featured version can be uploaded to SPIFFS later
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PlotterBot</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            background: #667eea;
            padding: 20px;
        }
        .container {
            max-width: 900px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        header {
            background: #764ba2;
            color: white;
            padding: 20px;
            text-align: center;
        }
        .content {
            padding: 20px;
        }
        canvas {
            border: 2px solid #333;
            cursor: crosshair;
            display: block;
            margin: 20px auto;
            touch-action: none;
        }
        .tools {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin-bottom: 20px;
        }
        button {
            padding: 10px 20px;
            border: none;
            background: #667eea;
            color: white;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
        }
        button:hover { background: #764ba2; }
        button.active { background: #764ba2; }
        .status {
            margin-top: 20px;
            padding: 10px;
            background: #f0f0f0;
            border-radius: 5px;
        }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🖊️ PlotterBot</h1>
            <p>WiFi Pen Plotter Interface</p>
        </header>
        <div class="content">
            <div class="tools">
                <button onclick="setTool('draw')" id="btnDraw">✏️ Draw</button>
                <button onclick="setTool('line')">📏 Line</button>
                <button onclick="setTool('rect')">⬜ Rectangle</button>
                <button onclick="setTool('circle')">⭕ Circle</button>
                <button onclick="clearCanvas()">🗑️ Clear</button>
            </div>

            <canvas id="canvas" width="600" height="400"></canvas>

            <div class="controls">
                <button onclick="sendToPlotter()" style="grid-column: 1 / -1; font-size: 18px;">
                    🚀 Send to Plotter
                </button>
                <button onclick="homeMotors()">🏠 Home</button>
                <button onclick="penUp()">⬆️ Pen Up</button>
                <button onclick="penDown()">⬇️ Pen Down</button>
                <button onclick="getStatus()">📊 Status</button>
            </div>

            <div class="status" id="status">
                Ready to draw! Select a tool and start creating.
            </div>
        </div>
    </div>

    <script>
        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        let currentTool = 'draw';
        let isDrawing = false;
        let startX, startY;
        let shapes = [];
        let currentPath = [];

        // Set drawing tool
        function setTool(tool) {
            currentTool = tool;
            document.querySelectorAll('.tools button').forEach(b => b.classList.remove('active'));
            document.getElementById('btnDraw').classList.remove('active');
            event.target.classList.add('active');
        }

        // Mouse/touch events
        canvas.addEventListener('mousedown', startDrawing);
        canvas.addEventListener('mousemove', draw);
        canvas.addEventListener('mouseup', stopDrawing);
        canvas.addEventListener('touchstart', handleTouch);
        canvas.addEventListener('touchmove', handleTouch);
        canvas.addEventListener('touchend', stopDrawing);

        function getCoords(e) {
            const rect = canvas.getBoundingClientRect();
            const x = (e.clientX || e.touches[0].clientX) - rect.left;
            const y = (e.clientY || e.touches[0].clientY) - rect.top;
            return {x, y};
        }

        function handleTouch(e) {
            e.preventDefault();
            if (e.type === 'touchstart') startDrawing(e);
            else if (e.type === 'touchmove') draw(e);
        }

        function startDrawing(e) {
            isDrawing = true;
            const coords = getCoords(e);
            startX = coords.x;
            startY = coords.y;
            if (currentTool === 'draw') {
                currentPath = [{x: startX, y: startY}];
            }
        }

        function draw(e) {
            if (!isDrawing) return;
            const coords = getCoords(e);

            if (currentTool === 'draw') {
                currentPath.push({x: coords.x, y: coords.y});
                redrawCanvas();
                ctx.strokeStyle = '#000';
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.moveTo(currentPath[0].x, currentPath[0].y);
                for (let i = 1; i < currentPath.length; i++) {
                    ctx.lineTo(currentPath[i].x, currentPath[i].y);
                }
                ctx.stroke();
            }
        }

        function stopDrawing(e) {
            if (!isDrawing) return;
            isDrawing = false;

            // For touch events, we might not have coordinates in stopDrawing
            let endX = startX;
            let endY = startY;

            if (e && (e.clientX || e.clientY || (e.touches && e.touches.length > 0))) {
                const coords = getCoords(e);
                endX = coords.x;
                endY = coords.y;
            } else if (e && e.changedTouches && e.changedTouches.length > 0) {
                const rect = canvas.getBoundingClientRect();
                endX = e.changedTouches[0].clientX - rect.left;
                endY = e.changedTouches[0].clientY - rect.top;
            }

            if (currentTool === 'draw' && currentPath.length > 1) {
                shapes.push({type: 'path', points: currentPath});
                const msg = 'Freehand path added (' + currentPath.length + ' points). Total shapes: ' + shapes.length;
                updateStatus(msg);
            } else if (currentTool === 'line') {
                shapes.push({type: 'line', x1: startX, y1: startY, x2: endX, y2: endY});
                const msg = 'Line added. Total shapes: ' + shapes.length;
                updateStatus(msg);
            } else if (currentTool === 'rect') {
                shapes.push({type: 'rect', x: startX, y: startY, w: endX - startX, h: endY - startY});
                const msg = 'Rectangle added. Total shapes: ' + shapes.length;
                updateStatus(msg);
            } else if (currentTool === 'circle') {
                const r = Math.sqrt(Math.pow(endX - startX, 2) + Math.pow(endY - startY, 2));
                shapes.push({type: 'circle', x: startX, y: startY, r: r});
                const msg = 'Circle added (radius: ' + Math.round(r) + 'px). Total shapes: ' + shapes.length;
                updateStatus(msg);
            }

            currentPath = [];
            redrawCanvas();
            console.log('Total shapes:', shapes.length);
        }

        function redrawCanvas() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.strokeStyle = '#000';
            ctx.lineWidth = 2;

            shapes.forEach(shape => {
                ctx.beginPath();
                if (shape.type === 'line') {
                    ctx.moveTo(shape.x1, shape.y1);
                    ctx.lineTo(shape.x2, shape.y2);
                } else if (shape.type === 'rect') {
                    ctx.rect(shape.x, shape.y, shape.w, shape.h);
                } else if (shape.type === 'circle') {
                    ctx.arc(shape.x, shape.y, shape.r, 0, Math.PI * 2);
                } else if (shape.type === 'path') {
                    ctx.moveTo(shape.points[0].x, shape.points[0].y);
                    for (let i = 1; i < shape.points.length; i++) {
                        ctx.lineTo(shape.points[i].x, shape.points[i].y);
                    }
                }
                ctx.stroke();
            });
        }

        function clearCanvas() {
            shapes = [];
            redrawCanvas();
            updateStatus('Canvas cleared');
        }

        // Generate G-code from shapes
        function generateGCode() {
            let gcode = '; PlotterBot G-code\n';
            gcode += 'G90\n';  // Absolute positioning
            gcode += 'G28\n';  // Home

            const scaleX = 200 / canvas.width;   // Scale to 200mm work area
            const scaleY = 200 / canvas.height;

            shapes.forEach(shape => {
                if (shape.type === 'line') {
                    gcode += `G0 X${(shape.x1 * scaleX).toFixed(2)} Y${(shape.y1 * scaleY).toFixed(2)}\n`;
                    gcode += `M3\n`;  // Pen down
                    gcode += `G1 X${(shape.x2 * scaleX).toFixed(2)} Y${(shape.y2 * scaleY).toFixed(2)}\n`;
                    gcode += `M5\n`;  // Pen up
                } else if (shape.type === 'rect') {
                    const x1 = shape.x * scaleX;
                    const y1 = shape.y * scaleY;
                    const x2 = (shape.x + shape.w) * scaleX;
                    const y2 = (shape.y + shape.h) * scaleY;
                    gcode += `G0 X${x1.toFixed(2)} Y${y1.toFixed(2)}\nM3\n`;
                    gcode += `G1 X${x2.toFixed(2)} Y${y1.toFixed(2)}\n`;
                    gcode += `G1 X${x2.toFixed(2)} Y${y2.toFixed(2)}\n`;
                    gcode += `G1 X${x1.toFixed(2)} Y${y2.toFixed(2)}\n`;
                    gcode += `G1 X${x1.toFixed(2)} Y${y1.toFixed(2)}\nM5\n`;
                } else if (shape.type === 'circle') {
                    const cx = shape.x * scaleX;
                    const cy = shape.y * scaleY;
                    const r = shape.r * Math.min(scaleX, scaleY);
                    const steps = 36;
                    gcode += `G0 X${(cx + r).toFixed(2)} Y${cy.toFixed(2)}\nM3\n`;
                    for (let i = 1; i <= steps; i++) {
                        const angle = (i / steps) * Math.PI * 2;
                        const x = cx + r * Math.cos(angle);
                        const y = cy + r * Math.sin(angle);
                        gcode += `G1 X${x.toFixed(2)} Y${y.toFixed(2)}\n`;
                    }
                    gcode += `M5\n`;
                } else if (shape.type === 'path') {
                    gcode += `G0 X${(shape.points[0].x * scaleX).toFixed(2)} Y${(shape.points[0].y * scaleY).toFixed(2)}\nM3\n`;
                    for (let i = 1; i < shape.points.length; i++) {
                        gcode += `G1 X${(shape.points[i].x * scaleX).toFixed(2)} Y${(shape.points[i].y * scaleY).toFixed(2)}\n`;
                    }
                    gcode += `M5\n`;
                }
            });

            gcode += 'M5\n';   // Pen up
            gcode += 'G28\n';  // Home when done
            return gcode;
        }

        // Send G-code to plotter
        async function sendToPlotter() {
            console.log('sendToPlotter called, shapes:', shapes.length);

            if (shapes.length === 0) {
                updateStatus('⚠️ Nothing to plot! Draw something first.');
                return;
            }

            updateStatus('📤 Generating G-code...');
            const gcode = generateGCode();
            console.log('Generated G-code:', gcode.substring(0, 200) + '...');

            updateStatus('📤 Sending ' + shapes.length + ' shape(s) to plotter...');

            try {
                const response = await fetch('/gcode', {
                    method: 'POST',
                    headers: {'Content-Type': 'text/plain'},
                    body: gcode
                });

                console.log('Response status:', response.status);
                const result = await response.text();
                console.log('Response:', result);

                updateStatus('✅ ' + result);
            } catch (error) {
                console.error('Error:', error);
                updateStatus('❌ Error: ' + error.message);
                alert('Error sending to plotter: ' + error.message);
            }
        }

        // Control functions
        async function homeMotors() {
            updateStatus('🏠 Homing...');
            await fetch('/gcode', {method: 'POST', body: 'G28'});
            updateStatus('✅ Homed');
        }

        async function penUp() {
            await fetch('/gcode', {method: 'POST', body: 'M5'});
            updateStatus('⬆️ Pen up');
        }

        async function penDown() {
            await fetch('/gcode', {method: 'POST', body: 'M3'});
            updateStatus('⬇️ Pen down');
        }

        async function getStatus() {
            const response = await fetch('/status');
            const status = await response.json();
            updateStatus(`Position: X=${status.x} Y=${status.y} Z=${status.z}`);
        }

        function updateStatus(msg) {
            document.getElementById('status').textContent = msg;
        }

        // Initialize
        document.getElementById('btnDraw').classList.add('active');
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
