set -u -e

# Compile it and output to executable called 'app'
g++ -std=c++11 -g bfs.cpp -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2 -o app
# runs the app
 ./app

