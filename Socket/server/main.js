const net = require('net');
const bcrypt = require('bcrypt');

const PORT = 8080;

async function hashPassword(password) {
  const saltRounds = 10;

  try {
    // Hash the password
    const hashed = await bcrypt.hash(password, saltRounds);
    console.log('Hashed password:', hashed);

    // Later: verify the password
    const isMatch = await bcrypt.compare(password, hashed);
    console.log('Password match:', isMatch); // true
  } catch (err) {
    console.error('Error hashing password:', err);
  }
}

const server = net.createServer((socket) => {
  console.log('Client connected:', socket.remoteAddress + ':' + socket.remotePort);

  let mode = null;         // 'login' or 'create'
  let clientPseudo = null;

  // ask for login or create account
  socket.write('Type "L" to Login or "C" to Create an account:\n');

  socket.on('data', (data) => {
    const message = data.toString().trim();

    if (!mode) {
      switch (message.toUpperCase()) {
        case ('L'):
          mode = 'login';
          socket.write('You chose to Login. Enter your pseudo:\n');
          break;

        case ('C'):
          mode = 'create';
          socket.write('You chose to Create an account. Enter your pseudo:\n');
          break;

        default:
          socket.write('Invalid input. Please type "L" or "C":\n');
          break;
      }
      return;
    }

    // ask for pseudo
    if (!clientPseudo) {
      clientPseudo = message;
      console.log(`[${mode.toUpperCase()}] Client chose pseudo: ${clientPseudo}`);
      socket.write(`Welcome, ${clientPseudo}!\n`);
      return;
    }

    // Step 4: Handle normal messages
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
