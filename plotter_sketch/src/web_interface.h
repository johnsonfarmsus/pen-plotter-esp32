#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

// Simplified HTML interface embedded as a string
// Full-featured version can be uploaded to SPIFFS later
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>PlotterBot</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            background: #667eea;
            padding: 10px;
            overflow-x: hidden;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        header {
            background: #764ba2;
            color: white;
            padding: 15px;
            text-align: center;
        }
        header h1 { font-size: 24px; }
        header p { font-size: 14px; margin-top: 5px; }
        .content {
            padding: 15px;
        }
        #canvasContainer {
            position: relative;
            width: 100%;
            max-width: 500px;
            margin: 15px auto;
        }
        #canvasContainer::before {
            content: "";
            display: block;
            padding-top: 100%; /* 1:1 Aspect Ratio */
        }
        canvas {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            border: 2px solid #333;
            cursor: crosshair;
            touch-action: none;
        }
        .tools {
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
            margin-bottom: 15px;
            justify-content: center;
        }
        button {
            padding: 12px 16px;
            border: none;
            background: #667eea;
            color: white;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            flex: 1;
            min-width: 120px;
        }
        button:hover { background: #764ba2; }
        button:active { background: #5a3a7a; }
        button.active { background: #764ba2; box-shadow: inset 0 2px 4px rgba(0,0,0,0.3); }
        .status {
            margin-top: 15px;
            padding: 12px;
            background: #f0f0f0;
            border-radius: 5px;
            font-size: 13px;
            min-height: 40px;
        }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
            margin-top: 15px;
        }
        .controls button.full-width {
            grid-column: 1 / -1;
            font-size: 16px;
            padding: 14px;
        }
        .text-controls {
            margin-top: 15px;
            display: none;
        }
        .text-controls.active {
            display: block;
        }
        input[type="text"], select, input[type="file"] {
            width: 100%;
            padding: 10px;
            margin: 8px 0;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }
        label {
            display: block;
            margin-top: 10px;
            font-weight: bold;
            font-size: 13px;
        }
        .upload-section {
            margin-top: 15px;
            padding: 15px;
            background: #f8f8f8;
            border-radius: 5px;
        }
        @media (max-width: 600px) {
            body { padding: 5px; }
            .content { padding: 10px; }
            button { font-size: 13px; padding: 10px 12px; min-width: 100px; }
            header h1 { font-size: 20px; }
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
                <button onclick="setTool('draw')" id="btnDraw" class="active">✏️ Draw</button>
                <button onclick="setTool('text')" id="btnText">📝 Text</button>
                <button onclick="clearCanvas()">🗑️ Clear</button>
            </div>

            <div class="text-controls" id="textControls">
                <label for="textInput">Enter Text (press Enter for new line):</label>
                <textarea id="textInput" placeholder="Type your text here...&#10;Press Enter for new lines" rows="4"></textarea>

                <label for="textSize">Text Size:</label>
                <select id="textSize">
                    <option value="24">Small (24px)</option>
                    <option value="36" selected>Medium (36px)</option>
                    <option value="48">Large (48px)</option>
                    <option value="64">Extra Large (64px)</option>
                </select>

                <button onclick="addTextToCanvas()" style="margin-top: 10px; width: 100%;">➕ Add Text to Canvas</button>
            </div>

            <div id="canvasContainer">
                <canvas id="canvas" width="500" height="500"></canvas>
            </div>

            <div class="upload-section">
                <label for="fileUpload">📁 Upload G-code or SVG File:</label>
                <input type="file" id="fileUpload" accept=".gcode,.nc,.svg,.txt" onchange="handleFileUpload(event)">
            </div>

            <div class="controls">
                <button onclick="sendToPlotter()" class="full-width">
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

        // Hershey stick font data (Block style - simplified single-line fonts for plotting)
        // Format: character -> array of stroke segments, each segment is array of [x,y] points
        // Coordinates are relative to character origin, normalized to ~21 unit height
        const blockFont = {
                'A': [[[0,21],[7,0]],[[14,21],[7,0]],[[3,7],[11,7]]],
                'B': [[[0,21],[0,0]],[[0,21],[9,21],[12,20],[13,19],[14,17],[14,15],[13,13],[12,12],[9,11]],[[0,11],[9,11],[12,10],[13,9],[14,7],[14,4],[13,2],[12,1],[9,0],[0,0]]],
                'C': [[[15,18],[14,20],[12,21],[9,21],[7,20],[5,18],[4,16],[3,13],[3,8],[4,5],[5,3],[7,1],[9,0],[12,0],[14,1],[15,3]]],
                'D': [[[0,21],[0,0]],[[0,21],[9,21],[12,20],[14,18],[15,16],[16,13],[16,8],[15,5],[14,3],[12,1],[9,0],[0,0]]],
                'E': [[[0,21],[0,0]],[[0,21],[15,21]],[[0,11],[12,11]],[[0,0],[15,0]]],
                'F': [[[0,21],[0,0]],[[0,21],[15,21]],[[0,11],[12,11]]],
                'G': [[[15,18],[14,20],[12,21],[9,21],[7,20],[5,18],[4,16],[3,13],[3,8],[4,5],[5,3],[7,1],[9,0],[12,0],[14,1],[15,3],[15,6],[14,8],[12,9],[9,9]]],
                'H': [[[0,21],[0,0]],[[16,21],[16,0]],[[0,11],[16,11]]],
                'I': [[[0,21],[0,0]]],
                'J': [[[12,21],[12,5],[11,2],[9,1],[7,0],[5,0],[3,1],[2,2],[1,5]]],
                'K': [[[0,21],[0,0]],[[16,21],[0,7]],[[6,12],[16,0]]],
                'L': [[[0,21],[0,0]],[[0,0],[14,0]]],
                'M': [[[0,21],[0,0]],[[0,21],[8,0]],[[16,21],[8,0]],[[16,21],[16,0]]],
                'N': [[[0,21],[0,0]],[[0,21],[16,0]],[[16,21],[16,0]]],
                'O': [[[7,21],[5,20],[3,18],[2,16],[1,13],[1,8],[2,5],[3,3],[5,1],[7,0],[9,0],[11,1],[13,3],[14,5],[15,8],[15,13],[14,16],[13,18],[11,20],[9,21],[7,21]]],
                'P': [[[0,21],[0,0]],[[0,21],[9,21],[12,20],[13,19],[14,17],[14,13],[13,11],[12,10],[9,9],[0,9]]],
                'Q': [[[7,21],[5,20],[3,18],[2,16],[1,13],[1,8],[2,5],[3,3],[5,1],[7,0],[9,0],[11,1],[13,3],[14,5],[15,8],[15,13],[14,16],[13,18],[11,20],[9,21],[7,21]],[[11,3],[15,-3]]],
                'R': [[[0,21],[0,0]],[[0,21],[9,21],[12,20],[13,19],[14,17],[14,14],[13,12],[12,11],[9,10],[0,10]],[[9,10],[16,0]]],
                'S': [[[15,18],[14,20],[11,21],[8,21],[5,20],[4,18],[5,16],[8,15],[11,15],[14,14],[15,12],[15,9],[14,7],[11,6],[8,6],[5,7],[4,9]]],
                'T': [[[0,21],[16,21]],[[8,21],[8,0]]],
                'U': [[[0,21],[0,6],[1,3],[3,1],[6,0],[10,0],[13,1],[15,3],[16,6],[16,21]]],
                'V': [[[0,21],[8,0]],[[16,21],[8,0]]],
                'W': [[[0,21],[4,0]],[[8,21],[4,0]],[[8,21],[12,0]],[[16,21],[12,0]]],
                'X': [[[0,21],[16,0]],[[16,21],[0,0]]],
                'Y': [[[0,21],[8,11]],[[16,21],[8,11]],[[8,11],[8,0]]],
                'Z': [[[0,21],[16,21]],[[16,21],[0,0]],[[0,0],[16,0]]],
                ' ': []
        };

        // Convert text string to plottable paths using Block font
        function textToPaths(text, size, startX, startY) {
            const font = blockFont;
            const scale = size / 21;  // Normalize to font height
            const charSpacing = size * 0.8;  // Character spacing
            const lineHeight = size * 1.5;  // Line height for multi-line text
            let paths = [];

            // Split text into lines (handle both \n and \r\n)
            const lines = text.split(/\r?\n/);

            // Render each line
            lines.forEach((line, lineIndex) => {
                const y = startY + (lineIndex * lineHeight);
                let xOffset = 0;

                for (let i = 0; i < line.length; i++) {
                    const char = line[i].toUpperCase();
                    const glyphStrokes = font[char] || font[' '];

                    glyphStrokes.forEach(stroke => {
                        const points = stroke.map(pt => ({
                            x: startX + (pt[0] + xOffset) * scale,
                            y: y - pt[1] * scale  // Invert Y (canvas Y goes down, font Y goes up)
                        }));

                        if (points.length > 1) {
                            paths.push({type: 'path', points: points});
                        }
                    });

                    xOffset += charSpacing;
                }
            });

            return paths;
        }

        // Set drawing tool
        function setTool(tool) {
            currentTool = tool;
            document.querySelectorAll('.tools button').forEach(b => b.classList.remove('active'));
            event.target.classList.add('active');

            // Show/hide text controls
            const textControls = document.getElementById('textControls');
            if (tool === 'text') {
                textControls.classList.add('active');
            } else {
                textControls.classList.remove('active');
            }
        }

        // Add text to canvas
        function addTextToCanvas() {
            const text = document.getElementById('textInput').value;
            const size = parseInt(document.getElementById('textSize').value);

            if (!text) {
                updateStatus('⚠️ Please enter some text first');
                return;
            }

            // Convert text to plottable paths using Block font
            // Start at (50, 80) - user controls line breaks via textarea
            const textPaths = textToPaths(text, size, 50, 80);

            // Add each path as a separate shape
            textPaths.forEach(path => shapes.push(path));

            redrawCanvas();
            updateStatus('✅ Text added: "' + text + '" (' + textPaths.length + ' strokes)');
            document.getElementById('textInput').value = '';
        }

        // Handle file upload
        function handleFileUpload(event) {
            const file = event.target.files[0];
            if (!file) return;

            updateStatus('📂 Reading file: ' + file.name);
            const reader = new FileReader();

            reader.onload = function(e) {
                const content = e.target.result;

                if (file.name.endsWith('.svg')) {
                    parseSVG(content);
                } else {
                    // Assume it's G-code
                    uploadedGCode = content;
                    updateStatus('✅ G-code loaded: ' + file.name + ' (ready to send)');
                }
            };

            reader.readAsText(file);
        }

        let uploadedGCode = null;

        // Simple SVG parser with auto-scaling to fit canvas
        function parseSVG(svgContent) {
            try {
                const parser = new DOMParser();
                const svgDoc = parser.parseFromString(svgContent, 'image/svg+xml');
                const svgElement = svgDoc.querySelector('svg');
                const paths = svgDoc.querySelectorAll('path');

                if (paths.length === 0) {
                    updateStatus('⚠️ No paths found in SVG');
                    return;
                }

                // Get SVG viewBox or dimensions
                let svgWidth, svgHeight, svgX = 0, svgY = 0;
                const viewBox = svgElement.getAttribute('viewBox');

                if (viewBox) {
                    const [x, y, w, h] = viewBox.split(/\s+/).map(parseFloat);
                    svgX = x;
                    svgY = y;
                    svgWidth = w;
                    svgHeight = h;
                } else {
                    svgWidth = parseFloat(svgElement.getAttribute('width')) || 100;
                    svgHeight = parseFloat(svgElement.getAttribute('height')) || 100;
                }

                // Calculate scale to fit canvas (with 10% margin)
                const margin = 0.9;  // 90% of canvas size
                const scaleX = (canvas.width * margin) / svgWidth;
                const scaleY = (canvas.height * margin) / svgHeight;
                const scale = Math.min(scaleX, scaleY);  // Maintain aspect ratio

                // Calculate centering offset
                const scaledWidth = svgWidth * scale;
                const scaledHeight = svgHeight * scale;
                const offsetX = (canvas.width - scaledWidth) / 2 - (svgX * scale);
                const offsetY = (canvas.height - scaledHeight) / 2 - (svgY * scale);

                // Create a temporary SVG to transform paths
                const tempSvg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
                tempSvg.setAttribute('width', canvas.width);
                tempSvg.setAttribute('height', canvas.height);

                paths.forEach(path => {
                    const d = path.getAttribute('d');
                    if (d) {
                        // Create a new path with transformation
                        const newPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
                        newPath.setAttribute('d', d);
                        newPath.setAttribute('transform', `translate(${offsetX}, ${offsetY}) scale(${scale})`);

                        // Get the transformed path data
                        tempSvg.appendChild(newPath);
                        const bbox = newPath.getBBox();
                        const transformedD = newPath.getAttribute('d');

                        // Apply transformation matrix to get actual coordinates
                        const matrix = newPath.getCTM();

                        shapes.push({
                            type: 'svg',
                            pathData: d,
                            transform: `translate(${offsetX}, ${offsetY}) scale(${scale})`
                        });
                    }
                });

                redrawCanvas();
                updateStatus('✅ SVG loaded and scaled to fit (' + paths.length + ' paths)');
            } catch (error) {
                updateStatus('❌ Error parsing SVG: ' + error.message);
            }
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
            const scaleX = canvas.width / rect.width;
            const scaleY = canvas.height / rect.height;
            const x = ((e.clientX || e.touches[0].clientX) - rect.left) * scaleX;
            const y = ((e.clientY || e.touches[0].clientY) - rect.top) * scaleY;
            return {x, y};
        }

        function handleTouch(e) {
            e.preventDefault();
            if (e.type === 'touchstart') startDrawing(e);
            else if (e.type === 'touchmove') draw(e);
        }

        function startDrawing(e) {
            if (currentTool !== 'draw') return;

            isDrawing = true;
            const coords = getCoords(e);
            startX = coords.x;
            startY = coords.y;
            currentPath = [{x: startX, y: startY}];
        }

        function draw(e) {
            if (!isDrawing || currentTool !== 'draw') return;
            const coords = getCoords(e);

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

        function stopDrawing(e) {
            if (!isDrawing) return;
            isDrawing = false;

            if (currentTool === 'draw' && currentPath.length > 1) {
                shapes.push({type: 'path', points: currentPath});
                const msg = 'Freehand path added (' + currentPath.length + ' points). Total shapes: ' + shapes.length;
                updateStatus(msg);
            }

            currentPath = [];
            redrawCanvas();
        }

        function redrawCanvas() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.strokeStyle = '#000';
            ctx.lineWidth = 2;

            shapes.forEach(shape => {
                if (shape.type === 'path') {
                    ctx.beginPath();
                    ctx.moveTo(shape.points[0].x, shape.points[0].y);
                    for (let i = 1; i < shape.points.length; i++) {
                        ctx.lineTo(shape.points[i].x, shape.points[i].y);
                    }
                    ctx.stroke();
                } else if (shape.type === 'svg') {
                    ctx.save();

                    // Apply transform if present
                    if (shape.transform) {
                        const transformMatch = shape.transform.match(/translate\(([^,]+),\s*([^)]+)\)\s*scale\(([^)]+)\)/);
                        if (transformMatch) {
                            const tx = parseFloat(transformMatch[1]);
                            const ty = parseFloat(transformMatch[2]);
                            const scale = parseFloat(transformMatch[3]);
                            ctx.translate(tx, ty);
                            ctx.scale(scale, scale);
                        }
                    }

                    const path = new Path2D(shape.pathData);
                    ctx.stroke(path);
                    ctx.restore();
                }
            });
        }

        function clearCanvas() {
            shapes = [];
            uploadedGCode = null;
            redrawCanvas();
            updateStatus('Canvas cleared');
        }

        // Generate G-code from shapes
        function generateGCode() {
            // If uploaded G-code exists, use that instead
            if (uploadedGCode) {
                return uploadedGCode;
            }

            let gcode = '; PlotterBot G-code\n';
            gcode += 'G90\n';  // Absolute positioning
            gcode += 'G28\n';  // Home

            const scaleX = 200 / canvas.width;   // Scale to 200mm work area
            const scaleY = 200 / canvas.height;

            shapes.forEach(shape => {
                if (shape.type === 'path') {
                    // Move to start position with pen up
                    gcode += `G0 X${(shape.points[0].x * scaleX).toFixed(2)} Y${(shape.points[0].y * scaleY).toFixed(2)}\n`;
                    gcode += `M3\n`;  // Pen down

                    // Draw the path
                    for (let i = 1; i < shape.points.length; i++) {
                        gcode += `G1 X${(shape.points[i].x * scaleX).toFixed(2)} Y${(shape.points[i].y * scaleY).toFixed(2)}\n`;
                    }

                    gcode += `M5\n`;  // Pen up
                } else if (shape.type === 'svg') {
                    // Convert SVG path to points for plotting
                    const tempCanvas = document.createElement('canvas');
                    tempCanvas.width = canvas.width;
                    tempCanvas.height = canvas.height;
                    const tempCtx = tempCanvas.getContext('2d');

                    // Apply transform if present
                    if (shape.transform) {
                        const transformMatch = shape.transform.match(/translate\(([^,]+),\s*([^)]+)\)\s*scale\(([^)]+)\)/);
                        if (transformMatch) {
                            const tx = parseFloat(transformMatch[1]);
                            const ty = parseFloat(transformMatch[2]);
                            const scale = parseFloat(transformMatch[3]);
                            tempCtx.translate(tx, ty);
                            tempCtx.scale(scale, scale);
                        }
                    }

                    const path2D = new Path2D(shape.pathData);

                    // Sample points along the path
                    const points = samplePath2D(path2D, tempCtx);

                    if (points.length > 0) {
                        // Move to start position with pen up
                        gcode += `G0 X${(points[0].x * scaleX).toFixed(2)} Y${(points[0].y * scaleY).toFixed(2)}\n`;
                        gcode += `M3\n`;  // Pen down

                        // Draw the path
                        for (let i = 1; i < points.length; i++) {
                            gcode += `G1 X${(points[i].x * scaleX).toFixed(2)} Y${(points[i].y * scaleY).toFixed(2)}\n`;
                        }

                        gcode += `M5\n`;  // Pen up
                    }
                }
            });

            gcode += 'M5\n';   // Pen up
            gcode += 'G28\n';  // Home when done
            return gcode;
        }

        // Sample points from Path2D (approximation for G-code conversion)
        function samplePath2D(path, ctx) {
            const points = [];
            const resolution = 2;  // Sample every 2 pixels

            // Create a temporary canvas to trace the path
            for (let x = 0; x < canvas.width; x += resolution) {
                for (let y = 0; y < canvas.height; y += resolution) {
                    if (ctx.isPointInStroke(path, x, y)) {
                        points.push({x, y});
                    }
                }
            }

            return points;
        }

        // Send G-code to plotter
        async function sendToPlotter() {
            if (shapes.length === 0 && !uploadedGCode) {
                updateStatus('⚠️ Nothing to plot! Draw something or upload a file first.');
                return;
            }

            // Confirmation dialog to prevent accidental multiple sends
            if (!confirm('Send drawing to plotter?\n\nThis will start plotting immediately. Make sure the plotter is ready.')) {
                updateStatus('❌ Plot cancelled');
                return;
            }

            updateStatus('📤 Generating G-code...');
            const gcode = generateGCode();

            updateStatus('📤 Sending to plotter...');

            try {
                const response = await fetch('/gcode', {
                    method: 'POST',
                    headers: {'Content-Type': 'text/plain'},
                    body: gcode
                });

                const result = await response.text();
                updateStatus('✅ ' + result);
            } catch (error) {
                updateStatus('❌ Error: ' + error.message);
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
        setTool('draw');
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
