const fs = require('fs');
const path = require('path');

const SAVE_FILE = path.join(__dirname, 'game_state.json');

let gameState = {
    health: 100,
    food: 100,
    isReviving: false,
    reviveProgress: 0,
    reviveTimeRemaining: "60:00",
    breathOffset: 0,
    breathDirection: 1,
    lastFedTime: Date.now(),
    lastHealthCheck: Date.now(),
    deathCount: 0,
    dayStreak: 0,
    lastDeathTime: null
};

// ** Load Game State **
function loadGameState() {
    if (fs.existsSync(SAVE_FILE)) {
        try {
            const data = JSON.parse(fs.readFileSync(SAVE_FILE, 'utf8'));
            Object.assign(gameState, data);
            console.log("📂 Game state loaded.");
            if (gameState.isReviving) resumeRevive();
        } catch (error) {
            console.error("❌ Failed to load game state:", error);
        }
    }
}

// ** Save Game State **
function saveGameState() {
    fs.writeFileSync(SAVE_FILE, JSON.stringify(gameState, null, 2));
}

// ** Apply Time-Based Depletion & Recovery **
function applyTimeBasedDepletion() {
    const now = Date.now();
    const elapsed = now - gameState.lastFedTime;

    // **Food Depletes Over Time (100 → 0 in 24h)**
    const foodCycles = Math.floor(elapsed / (60 * 1000)); // Every 1 min
    if (foodCycles > 0) {
        gameState.food = Math.max(0, gameState.food - foodCycles * (100 / 1440)); // 100 → 0 in 24h
        gameState.lastFedTime = now;
    }

    // **Health Regeneration (If Food > 50%)**
    while (gameState.food > 50 && gameState.health < 100) {
        gameState.food -= 3; // Consumes food
        gameState.health += 1; // Gains health
    }

    // **Health Decay Only When Food is 0**
    if (gameState.food === 0) {
        const healthCycles = Math.floor((now - gameState.lastHealthCheck) / (60 * 1000)); // Every min
        if (healthCycles > 0) {
            gameState.health = Math.max(0, gameState.health - healthCycles * 1); // 1% per min
            gameState.lastHealthCheck = now;
        }
    }

    saveGameState();
}

// ** Breathing Animation Update **
function updateBreathing() {
    gameState.breathOffset += gameState.breathDirection;
    if (gameState.breathOffset >= 2 || gameState.breathOffset <= -2) {
        gameState.breathDirection *= -1;
    }
}

// ** Revive Countdown Timer **
function getReviveTimeRemaining() {
    if (!gameState.isReviving) return "00:00";
    const elapsed = Date.now() - gameState.reviveStartTime;
    const remaining = Math.max(0, 60 * 60 * 1000 - elapsed); // 1-hour delay
    const minutes = Math.floor(remaining / 60000);
    const seconds = Math.floor((remaining % 60000) / 1000);
    return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
}

// ** Revive Progress Percentage (for UI) **
function getReviveProgress() {
    if (!gameState.isReviving) return 0;
    const elapsed = Date.now() - gameState.reviveStartTime;
    return Math.min(100, (elapsed / (60 * 60 * 1000)) * 100); // 1-hour full progress
}

// ** Game Tick **
function gameTick() {
    applyTimeBasedDepletion();
    updateBreathing();

    if (gameState.isReviving) {
        gameState.reviveTimeRemaining = getReviveTimeRemaining();
        if (gameState.reviveTimeRemaining === "00:00") revive();
    }

    saveGameState();
}

// ** Start Revive (1-Hour Timer) **
function startRevive() {
    if (gameState.health === 0 && !gameState.isReviving) {
        gameState.isReviving = true;
        gameState.reviveStartTime = Date.now();
        gameState.reviveTimeRemaining = "60:00";
        console.log("⏳ Reviving started... Time remaining: 60:00");
    }
}

// ** Resume Revive if Restarted **
function resumeRevive() {
    if (gameState.isReviving) {
        console.log("Resuming revive countdown...");
        gameState.reviveTimeRemaining = getReviveTimeRemaining();
    }
}

// ** Complete Revive Process **
function revive() {
    gameState.health = 50;
    gameState.food = 50;
    gameState.isReviving = false;
    gameState.reviveTimeRemaining = "00:00";
    console.log("✅ Pet revived! Health: 50, Food: 50");
    saveGameState();
}

// ** Feeding Mechanic **
function feedPet() {
    gameState.food = Math.min(100, gameState.food + 10);
    gameState.lastFedTime = Date.now();
    console.log(`🍗 Pet Fed! Food: ${gameState.food}`);
    saveGameState();
}

// ** Healing Mechanic **
function healPet() {
    gameState.health = Math.min(100, gameState.health + 10);
    console.log(`❤️ Pet Healed! Health: ${gameState.health}`);
    saveGameState();
}

// ** Get Current Game State **
function getGameState() {
    return { 
        health: gameState.health,
        food: gameState.food,
        breathOffset: gameState.breathOffset,
        isReviving: gameState.isReviving,
        reviveTimeRemaining: getReviveTimeRemaining(),
        reviveProgress: getReviveProgress(),
        deathCount: gameState.deathCount,
        dayStreak: gameState.dayStreak
    };
}

// ** Load game state on startup **
loadGameState();
applyTimeBasedDepletion();
setInterval(gameTick, 1000);

// ** Export functions for `main.js` **
module.exports = { 
    gameTick, 
    startRevive,
    revive,
    feedPet,
    healPet,
    getGameState: () => ({
        health: gameState.health,
        food: gameState.food,
        breathOffset: gameState.breathOffset,
        isReviving: gameState.isReviving,
        reviveProgress: gameState.reviveProgress,
        deathCount: gameState.deathCount,
        reviveTimeRemaining: getReviveTimeRemaining(),
        dayStreak: gameState.dayStreak
    })
};
