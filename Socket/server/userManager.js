const fs = require('fs');
const bcrypt = require('bcrypt');
const filePath = './users.json';

let users = fs.existsSync(filePath) ? JSON.parse(fs.readFileSync(filePath, 'utf8')) : {};

function saveUsers() {
  fs.writeFileSync(filePath, JSON.stringify(users, null, 2));
}

function userExists(username) {
  return !!users[username];
}

async function createUser(username, rawPassword) {
  if (!username || !rawPassword || userExists(username)) return false;
  const hash = await bcrypt.hash(rawPassword, 10);
  users[username] = { password: hash, publicKey: null };  // Initialize publicKey field
  saveUsers();
  return true;
}

async function validateLogin(username, rawPassword) {
  const user = users[username];
  if (!user) return false;
  return await bcrypt.compare(rawPassword, user.password);
}

// NEW: Save user's public key (base64 string)
function setPublicKey(username, base64Key) {
  if (!users[username]) return false;
  users[username].publicKey = base64Key;
  saveUsers();
  return true;
}

// NEW: Get user's public key (base64 string)
function getPublicKey(username) {
  if (!users[username]) return null;
  return users[username].publicKey;
}

module.exports = {
  userExists,
  createUser,
  validateLogin,
  setPublicKey,
  getPublicKey,
  cleanOLdMessages
};
