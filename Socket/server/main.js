const net = require('net');
const { userExists, createUser, validateLogin, getPublicKey, setPublicKey } = require('./userManager');

const PORT = 8080;

const clients = new Map(); // username -> socket
const currentChats = new Map(); // username -> target username for private chat

const server = net.createServer((socket) => {
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
      socket.username = username;

      clients.set(username, socket);

      return socket.write(
        'You can now send messages.\n' +
        'Send your public key using:\nPUBKEY <your PEM-formatted key>\n' +
        'Use "/mp <username>" to start a private conversation.\n'
      );
    }

    if (mode === 'chat' && loggedIn) {
      if (msg.startsWith('PUBKEY ')) {
        const raw = msg.slice(7);
        const keyBase64 = raw
          .replace(/-----(BEGIN|END) PUBLIC KEY-----/g, '')
          .replace(/\r?\n|\r/g, '');
        if (setPublicKey(username, keyBase64)) {
          return socket.write('Public key saved - ok.\n');
        }
        return socket.write('Public key saved - error.\n');
      }

      if (msg.startsWith('/mp ')) {
        const target = msg.split(' ')[1];
        if (!target) {
          return socket.write('Usage: /mp <username>\n');
        }

        if (!userExists(target)) {
          return socket.write(`ERROR: User ${target} does not exist.\n`);
        }

        const pubKeyBase64 = getPublicKey(target);
        if (!pubKeyBase64) {
          return socket.write(`ERROR: Public key for ${target} not found.\n`);
        }

        const targetPEM = `-----BEGIN PUBLIC KEY-----\n${pubKeyBase64}\n-----END PUBLIC KEY-----`;

        currentChats.set(username, target);

        socket.write(`TARGET_PUBLIC_KEY ${target} ${targetPEM}\n`);
        socket.write(`Private chat started with ${target}. Encrypt your messages with this key.\n`);
        return;
      }

      // Normal message sent by client — treat as encrypted message to current private chat target
      const target = currentChats.get(username);
      if (!target) {
        return socket.write('No private chat target set. Use /mp <username> to start one.\n');
      }

      const targetSocket = clients.get(target);
      if (!targetSocket) {
        return socket.write(`ERROR: User ${target} not online.\n`);
      }

      // Forward the encrypted message to the target
      targetSocket.write(`MP_FROM ${username} ${msg}\n`);
      socket.write(`MP_SENT to ${target}\n`);
      return;
    }
  });

  socket.on('end', () => {
    if (loggedIn && username) {
      clients.delete(username);
      currentChats.delete(username);
      console.log(`Client ${username} disconnected`);
    } else {
      console.log(`Client disconnected`);
    }
  });

  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

server.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});

