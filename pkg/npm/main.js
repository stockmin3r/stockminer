const { app, BrowserWindow } = require('electron');
const { spawn }              = require('child_process');
const os = require("os")

const homedir = os.homedir();

app.disableHardwareAcceleration();

function createWindow() {
	const mainWindow = new BrowserWindow({
		width:1280,
		height:720,
		webPreferences: {
			nodeIntegration: false,
			contextIsolation: false
		}
	})
	mainWindow.loadURL("https://localhost:4443");
  
	mainWindow.webContents.on("did-fail-load", function() {
		mainWindow.loadURL("https://localhost:4443");
  });
}

app.on('certificate-error', (event, webContents, url, error, certificate, callback) => {
  event.preventDefault();
  callback(true);
});

app.whenReady().then(() => {
	spawn(homedir + "/.config/stockminer/bin/stockminer", ['-d'])
	createWindow();
});

app.on('activate', () => {
	if (BrowserWindow.getAllWindows().length == 0)
		createWindow();
});
