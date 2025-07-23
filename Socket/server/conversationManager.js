const fs = require('fs');
const path = require('path');

const convDir = path.resolve(__dirname, 'conversations');
if (!fs.existsSync(convDir)) {
  fs.mkdirSync(convDir);
}

// Helper: get conversation filename for two users (sorted)
function getConversationFile(user1, user2) {
  const users = [user1, user2].sort();
  return path.join(convDir, `${users[0]}_${users[1]}.json`);
}

// Save a message in conversation between sender and receiver
function saveMessage(sender, receiver, encryptedMsg) {
  const file = getConversationFile(sender, receiver);
  let conversation = [];

  if (fs.existsSync(file)) {
    try {
      conversation = JSON.parse(fs.readFileSync(file, 'utf8'));
    } catch {
      conversation = [];
    }
  }

  // Save timestamp too
  conversation.push({
    sender,
    receiver,
    message: encryptedMsg,
    timestamp: Date.now(),
  });

  fs.writeFileSync(file, JSON.stringify(conversation, null, 2));
}

// Get and delete all pending messages for user
function getPendingMessages(username) {
  const files = fs.readdirSync(convDir);
  let pending = [];

  for (const file of files) {
    if (!file.endsWith('.json')) continue;

    const filePath = path.join(convDir, file);
    let conversation;

    try {
      conversation = JSON.parse(fs.readFileSync(filePath, 'utf8'));
    } catch {
      continue;
    }

    // Filter messages where receiver is the user
    const toDeliver = conversation.filter(msg => msg.receiver === username);

    // Keep messages not for the user (or older than 24h)
    const now = Date.now();
    const keepMessages = conversation.filter(msg => {
      // Keep if not for user OR if message is younger than 24h
      if (msg.receiver !== username) return true;
      // Remove if older than 24h (86400000 ms)
      return now - msg.timestamp < 86400000;
    });

    if (toDeliver.length > 0) {
      pending = pending.concat(toDeliver);

      // Save back only the kept messages (remove delivered older than 24h)
      fs.writeFileSync(filePath, JSON.stringify(keepMessages, null, 2));
    }
  }

  return pending;
}

module.exports = {
  saveMessage,
  getPendingMessages,
};
