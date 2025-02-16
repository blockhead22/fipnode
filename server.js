const express = require('express');
const path = require('path');

const app = express();
const PORT = 3000;

// Serve static files
app.use(express.static(path.join(__dirname, 'public')));

// Simulate button presses triggering pet state changes
let petState = "happy";
const clients = [];

app.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');

    clients.push(res);
    res.write(`data: ${petState}\n\n`);

    req.on('close', () => {
        clients.splice(clients.indexOf(res), 1);
    });
});

function updatePetState(newState) {
    petState = newState;
    clients.forEach(client => client.write(`data: ${newState}\n\n`));
}

// Simulating button inputs
setInterval(() => {
    updatePetState(petState === "happy" ? "hungry" : "happy");
}, 5000); // Toggle state every 5 seconds

// Start server
app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
