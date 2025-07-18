const net = require('net');
const {
  userExists,
  createUser,
  validateLogin
} = require('./userManager');

const PORT = 8080;

const server = net.createServer((socket) => {
  console.log('Client connected:', socket.remoteAddress);

  let mode = null;
  let step = null;
  let username = null;
  let loggedIn = false;

  socket.write('Type "L" to Login or "C" to Create an account:\n');

  socket.on('data', async (data) => {
    const msg = data.toString().trim();

    if (!mode) {
      if (msg === 'L' || msg === 'C') {
        mode = msg === 'L' ? 'login' : 'create';
        step = 'username';
        return socket.write('Enter username:\n');
      }
      return socket.write('Invalid input. Type "L" or "C":\n');
    }

    if (step === 'username') {
      username = msg;

      if (mode === 'create' && userExists(username))
        return socket.write('Username exists, try another:\n');

      if (mode === 'login' && !userExists(username))
        return socket.write('Username not found, try again:\n');

      step = 'password';
      return socket.write('Enter password:\n');
    }

    if (step === 'password') {
      const password = msg;

      if (mode === 'create') {
        const created = await createUser(username, password);
        if (!created) return socket.write('User creation failed.\n');
        socket.write(`Account created! Welcome, ${username}.\n`);
      } else {
        const valid = await validateLogin(username, password);
        if (!valid) return socket.write('Incorrect password. Try again:\n');
        socket.write(`Login successful! Welcome, ${username}.\n`);
      }

      loggedIn = true;
      mode = 'chat';
      step = null;
      return socket.write('You can now send messages.\n');
    }

    if (mode === 'chat' && loggedIn) {
      console.log(`[${username}] says: ${msg}`);
      socket.write(`You: ${msg}\n`);
    }
  });

  socket.on('end', () => console.log(`Client ${username || socket.remoteAddress} disconnected`));
  socket.on('error', (err) => console.error('Socket error:', err));
});

server.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});
