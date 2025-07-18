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
  users[username] = { password: hash };
  saveUsers();
  return true;
}

async function validateLogin(username, rawPassword) {
  const user = users[username];
  if (!user) return false;
  return await bcrypt.compare(rawPassword, user.password);
}

module.exports = {
  userExists,
  createUser,
  validateLogin
};
