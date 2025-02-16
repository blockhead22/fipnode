const fip = require('./build/Release/fip_addon');

console.log("Initializing FIP...");
if (fip.initializeFIP()) {
    console.log("FIP Ready!");
} else {
    console.log("Failed to initialize FIP.");
}
