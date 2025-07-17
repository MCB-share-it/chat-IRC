const net = require('net');

const PORT = 8080;

const server = net.createServer((socket) => {
    console.log('Client connected:', socket.remoteAddress + ':' + socket.remotePort);

    let clientPseudo = null;

    // Prompt for pseudo immediately after connection
    socket.write('Enter your pseudo:\n');

    socket.on('data', (data) => {
        const message = data.toString().trim();

        // If pseudo not set, first message is the pseudo
        if (!clientPseudo) {
            clientPseudo = message;
            console.log(`Client chose pseudo: ${clientPseudo}`);
            socket.write(`Welcome, ${clientPseudo}!\n`);
            return;
        }

        // Handle normal messages
        console.log(`[${clientPseudo}] says: ${message}`);
        socket.write(`${message}\n`);
    });

    socket.on('end', () => {
        console.log(`Client ${clientPseudo || socket.remoteAddress} disconnected`);
    });

    socket.on('error', (err) => {
        console.error('Socket error:', err);
    });
});

server.listen(PORT, '0.0.0.0', () => {
    console.log(`Server listening on port ${PORT}`);
});
