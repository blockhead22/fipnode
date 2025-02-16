const fs = require('fs');
const path = require('path');
const puppeteer = require('puppeteer');
const addon = require('./build/Release/fip_addon');
const express = require('express');
const { gameTick, feedPet, healPet, startRevive, getGameState, reviveProgress } = require('./game_logic'); // ✅ Re-add feedPet & healPet

const app = express();
const PORT = 3000;

console.log("Initializing DirectOutput...");

// Initialize DirectOutput
const result = addon.initDirectOutput();
console.log("Init result:", result);

if (result !== 0) {
    console.error("Failed to initialize DirectOutput. Exiting...");
    process.exit(1);
}

let buttonPressCount = 0;
let lastButtonPressed = "None";
let currentButtonState = 0x00000000;
let lastButtonState = 0x00000000; // ✅ Track last button state
let revivePressCount = 0; // ✅ Track revive presses
const requiredRevivePresses = 4; // ✅ Require 4 presses

// Serve static files
app.use(express.static(path.join(__dirname, 'public')));

app.get('/button-data', (req, res) => {
    const { health, food, isReviving, reviveTimeRemaining, breathOffset, deathCount, dayStreak } = getGameState();
    res.json({ 
        health, 
        food, 
        isReviving, 
        reviveTimeRemaining, 
        buttonState: `0x${currentButtonState.toString(16).toUpperCase()}`,
        count: buttonPressCount, 
        lastButton: lastButtonPressed, 
        breathOffset, 
        reviveProgress, 
        deathCount,
        dayStreak
    });
});


app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});

// ** Start Puppeteer Once Globally **
let browser;
let page;

async function startBrowser() {
    console.log("Launching Puppeteer...");
    browser = await puppeteer.launch({ headless: true, args: ['--no-sandbox', '--disable-setuid-sandbox'] });
    page = await browser.newPage();
    await page.setCacheEnabled(false);
    await page.setViewport({ width: 320, height: 240 });

    console.log("Puppeteer started.");
}

// Function to capture screenshot
async function captureScreenshot() {
    if (!browser || !page) {
        console.error("❌ Puppeteer not initialized.");
        return null;
    }

    try {
        console.log("Navigating to http://localhost:3000...");
        await page.goto(`http://localhost:${PORT}`, { waitUntil: 'networkidle2' });

        const screenshotPath = path.join(__dirname, 'screenshot.jpg');
        await page.screenshot({ path: screenshotPath });

        console.log(`Screenshot captured at ${new Date().toISOString()}`);
        return screenshotPath;
    } catch (error) {
        return null;
    }
}

// Function to send screenshot to FIP
async function sendScreenshotToFIP() {
    try {
        const screenshotPath = await captureScreenshot();
        if (!screenshotPath || !fs.existsSync(screenshotPath)) {
            throw new Error(`Screenshot file does not exist at path: ${screenshotPath}`);
        }

        const imageResult = addon.sendImageFromFile(screenshotPath);
        if (imageResult !== "") {
            console.error(`❌ Failed to send screenshot. Error: ${imageResult}`);
        }
    } catch (error) {}
}

// ** Start Puppeteer and Begin Capturing **
(async () => {
    await startBrowser();
    setInterval(sendScreenshotToFIP, 1200); // Capture every 3 seconds
})();

// ** Register Button Callback **
addon.registerButtonCallback((buttonId) => {
    currentButtonState = buttonId; // Store current button press
    lastButtonPressed = `0x${buttonId.toString(16).toUpperCase()}`;

    if (buttonId === 0x20) {
        feedPet(); // ✅ Button 0x20 adds food
        console.log("🍗 Pet fed!");
    } else if (buttonId === 0x02) {
        healPet(); // ✅ Button 0x02 heals the pet
        console.log("❤️ Pet healed!");
    }

    // ✅ Single press triggers revive process (1-hour delay)
    if (buttonId === 0x400) {
        startRevive();
        console.log("⏳ Revive started! Pet will revive in 1 hour.");
    }
});



// ** Gracefully Exit **
process.on('SIGINT', async () => {
    console.log("\nShutting down...");
    if (browser) await browser.close();
    process.exit(0);
});
