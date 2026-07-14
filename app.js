const canvasContext = document.getElementById('analyticsChart').getContext('2d');
const dataPointsLimit = 10;
const analyticsChart = new Chart(canvasContext, {
    type: 'line',
    data: {
        labels: Array(dataPointsLimit).fill(''),
        datasets: [{ label: 'Signals', data: Array(dataPointsLimit).fill(0), borderColor: '#00f2fe', borderWidth: 2, tension: 0.4, pointRadius: 0 }]
    },
    options: { responsive: true, maintainAspectRatio: false, scales: { x: { grid: { display: false } }, y: { grid: { color: 'rgba(255,255,255,0.05)' } } }, plugins: { legend: { display: false } } }
});
function writeToTerminal(logMessage, severity = 'info') {
    const logBox = document.getElementById('terminal-logs');
    const timestamp = new Date().toLocaleTimeString();
    const cleanLog = document.createElement('div');
    cleanLog.className = severity === 'danger' ? 'text-red-500 font-bold' : 'text-cyan-400/70';
    cleanLog.innerHTML = `[${timestamp}] // ${logMessage}`;
    logBox.appendChild(cleanLog);
}
window.triggerSystemOverride = function(type) {
    writeToTerminal(`Simulation shift triggered: ${type.toUpperCase()}`, type === 'fire' ? 'danger' : 'info');
}
writeToTerminal("SENTINEL-X core web-console initialized successfully.", "info");

