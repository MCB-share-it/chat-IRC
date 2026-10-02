# chat-IRC
chat irc project using different technologies

HUGE THANKS TO https://www.youtube.com/@CyberHashira, his tutorials helped me a lot using the ssl library



gcc -o client client.c crypto_utils.c conversationManager.c -pthread -lssl -lcrypto -ljanssson

library required : 
libssl-dev
