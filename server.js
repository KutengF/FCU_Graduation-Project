const express = require('express');
const bodyParser = require('body-parser');
const CryptoJS = require('crypto-js');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');

const app = express();
const PORT = 3000;

//const aesKey = '0000000000000000';
const aesKey = '2d35615a231a869f';
const aesIV = CryptoJS.enc.Hex.parse('00000000000000000000000000000000');
//const aesIV = CryptoJS.enc.Hex.parse('12345678901234567890123456789012');

let currentStatus = '';

app.use(bodyParser.text());
app.use(bodyParser.json()); 
app.use(express.static(path.join(__dirname, 'www')));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'www', 'index.html'));
});

app.post('/data', (req, res) => {
    console.log('Received encrypted data:', req.body);

    const decryptedBytes = CryptoJS.AES.decrypt(req.body, CryptoJS.enc.Utf8.parse(aesKey), {
        iv: aesIV
    });
    currentStatus = decryptedBytes.toString(CryptoJS.enc.Utf8);
    
    console.log('Decrypted data:', currentStatus);
    io.emit('updateStatus', currentStatus);

    res.json({ status: "success", message: "Data received and decrypted!" });
});

app.get('/getUpdates', (req, res) => {
    console.log("Sending update to ESP8266:", "FromServer:" + currentStatus);
    res.send("FromServer:" + currentStatus);
});

app.post('/update', (req, res) => {
    const data = req.body.data;
    currentStatus = data;
    console.log('Updated data (from index.html):', currentStatus);
    
    res.json({ status: "success", message: "Data updated!" });
});

app.post('/changeMode', (req, res) => {
    // Check the current mode and toggle it
    let currentMode = currentStatus[5]; // Assuming mode is at the 6th position
    currentMode = (currentMode === '0') ? '1' : '0';

    // Update the currentStatus with the new mode
    currentStatus = currentStatus.substring(0, 5) + currentMode + currentStatus.substring(6);

    console.log('Mode changed:', currentMode === '0' ? "Manual Mode" : "Auto Mode");
    
    // Notify the ESP8266 about the change
    io.emit('updateStatus', currentStatus);
    
    res.json({ status: "success", message: "Mode updated!" });
});

const server = http.createServer(app);
const io = socketIo(server);

io.on('connection', (socket) => {
    console.log('New client connected');
    socket.emit('updateStatus', currentStatus); 

    // Log when the status is updated from the client
    socket.on('updateStatus', (data) => {
        console.log("Received updateStatus event from client with data:", data);
    });

    // Handle the update event from client
    socket.on('update', (data) => {
        console.log("Received update from client:", data);
        currentStatus = data;
    });

    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
