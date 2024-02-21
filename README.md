# 1. General System Description

In this repo you can see implementation of a server-client chat application with file sharing capabilities. The application has a server component and a client component. The server handles multiple clients concurrently, and clients can join different rooms to exchange text messages and files.

### Server Component

The server uses a multithreaded design to handle incoming connections and messages from clients concurrently.
Communication between the server and clients is achieved through sockets.
Rooms are used to group clients, and the server maintains a list of connected rooms.

### Client Component

The client's command-line interface (CLI) allows users to enter a username, join a room, send/receive text messages, and send/receive files.
The client uses multithreading to simultaneously handle user input and incoming messages.

# 2. Application Protocol Description

The communication protocol is based on simple text messages exchanged between the server and clients. The messages include commands such as "/quit" to leave a room, "/file" to initiate file sharing, "/receive" to accept a file, "/reject" to reject a file, and regular text messages.

- Connect to the server that works on port `12345`
- Send your username using [sendMessage](#sendmessage) function
- Send room id you want to join (`4 bytes, int32`)
- To send message to another user in the same room use [sendMessage](#sendmessage) function
- To receive message use [receiveMessage](#receivemessage) function
- To share file you need to send:

  1.  "/file" command ([sendMessage](#sendmessage))
  2.  File name (with extension e.g file.txt) using [sendMessage](#sendmessage) function
  3.  File itself using [sendFileUsingChunks](#sendfileusingchunks) function
  4.  After all clients receive file, server will send you "/received" message

- To receive incoming file:
  1.  Send "/receive" command ([sendMessage](#sendmessage))
  2.  Send file name you want to receive ([sendMessage](#sendmessage))
  3.  Receive file itself using [receiveChunkedDataToFile](#receivechunkeddatatofile) function
- To reject incoming file send "/reject" command ([sendMessage](#sendmessage))
- To quit the room you need to send "/quit" command and in order to rejoin another room send room id you want to join (`4 bytes, int32`)

# 3. Screenshots of Different Use Cases

### Join Room:

- Client enters a username, room ID, and storage folder path.
  ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/8edfc5ed-c82d-498c-9cff-b417fb446701)

### Send Message

- Client sends message.
  ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/291f4cb7-06a8-471f-90c3-c9518f2deaa3)

### Send File:

- Client initiates file sending using the "/file" command.
   ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/0ff3f0af-e077-458a-a1bc-a4cba4a4b760)

### Accept File:

- Client accepts a received file using the "/receive" command.
 ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/237cc4fd-641f-44f6-bf46-3c6f25eaf49d)

### Decline File:

- Client declines a received file using the "/reject" command.
  ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/147b23b5-904d-4231-84c6-01650ad62ba1)

### Change Room:

- Client changes the current room by typing "/quit" and entering a new room ID.
  ![image](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/a974ba34-4111-4a42-89d6-298b648de177)

# 4. Explanation of Different Use Cases Using UML Diagrams and Code Examples

### Sequence Diagram
![chat_application_sequence_diagram](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/42fc3fb3-aa9b-43c5-8d23-b5e880833373)

### Use Case Diagram
![chat_application_usecase_diagram](https://github.com/danylo-gavrylovskyi/ChatApplication/assets/118884527/3be4fead-9ef7-423d-9f2b-63786f2ef1bd)

## Code Examples

### Server Code:

1. Handling Client Connection

```cpp
// Inside Server::start()
SOCKET clientSocket = serverSocket.acceptConnection();
if (clientSocket == INVALID_SOCKET) {
    // Handle connection failure
    std::lock_guard<std::mutex> lock(consoleMtx);
    std::cerr << "Accept failed.\n";
    serverSocket.closeConnection();
    return -1;
}
// Successfully accepted connection
{
    std::lock_guard<std::mutex> lock(consoleMtx);
    std::cout << "Client " << clientSocket << " connected.\n";
}
// Create a new thread to handle the client
std::thread clientThread([this, clientSocket, &fileHandler]() {
    handleClient(clientSocket, fileHandler);
});
clientThread.detach();
```

2. Handling Text Message

```cpp
// Inside Server::handleClient()
std::string receivedMessage = dataStreamer.receiveMessage(clientSocket);

if (receivedMessage == "/quit") {
    // Handle user leaving the current room
    getRoomById(roomId).removeClient(user);
    pushToQueue(Message{ "--- " + user.username + " left ---", user.clientSocket, roomId });
    roomId = dataStreamer.receiveInt(clientSocket);
    joinRoom(roomId, user);
}
else {
    // Handle regular text message
    std::string msgWithUsername = username + ": " + move(receivedMessage);
    Message msg{ move(msgWithUsername), user.clientSocket, roomId };
    pushToQueue(std::move(msg));
}
```

3. Handling File Transfer

```cpp
// Inside Server::handleClient()
else if (receivedMessage == "/file") {
    // Handle file transfer initiation
    pushToQueue(Message{ receivedMessage, user.clientSocket, roomId });
    std::string filename = dataStreamer.receiveMessage(clientSocket);
    std::string pathToFile = SERVER_STORAGE + filename;
    dataStreamer.receiveChunkedDataToFile(clientSocket, pathToFile, fileHandler);

    // Notify clients about the file and wait for responses
    Message sendFileMsg{ username + " wants to send " + move(filename) + " file. Do you want to receive? (/receive or /reject)", clientSocket, roomId };
    pushToQueue(sendFileMsg);

    int toBeReceived = getNumOfClients(roomId) - 1;
    std::unique_lock<std::mutex> lock(allFilesReceivedMtx);
    fileReceived.wait(lock, [this, toBeReceived]() { return numOfReceivedFiles == toBeReceived; });
    dataStreamer.sendMessage(clientSocket, "/received");
    numOfReceivedFiles = 0;
}
```

4. Handling File Acceptance

```cpp
// Inside Server::handleClient()
else if (receivedMessage == "/receive") {
    // Handle file acceptance
    std::string filename = dataStreamer.receiveMessage(clientSocket);
    dataStreamer.sendFileUsingChunks(clientSocket, SERVER_STORAGE + filename, 100000);
    incrementNumOfReceivedFiles(getNumOfClients(roomId) - 1);
}
```

5. Broadcasting General Messages

```cpp
// Inside Server::broadcastMessage()
for (const User& user : getRoomById(msg.roomId).getClients()) {
    if (user.clientSocket != msg.sender) {
        dataStreamer.sendMessage(user.clientSocket, msg.content);
    }
}
```

### Client Code:

1. Handling User Input

```cpp
// Inside CLI::run()
std::string message;
while (true) {
    std::getline(std::cin >> std::ws, message);
    // Handle user input based on different commands
    if (message == "/quit") {
        // Handle leaving the current room
        std::cout << "Enter room id you want to join: ";
        std::cin >> roomId;
        dataStreamer.sendIntData(clientSocket, roomId);
    }
    else if (message == "/file") {
        // Handle initiating file sending
        // ...
    }
    // Handle other cases
    dataStreamer.sendMessage(clientSocket, message);
}
```

2. Handling File Receiving

```cpp
// Inside CLI::run()
else if (message == "/receive") {
    dataStreamer.sendMessage(clientSocket, filename);
    dataStreamer.receiveChunkedDataToFile(clientSocket, pathToClientStorage + "/" + filename, fileHandler);
    std::cout << "File received.\n";

    std::lock_guard<std::mutex> lock(sendingFileMtx);
    isSendingFile = false;
}
```

# Functions to work with data

`All implementations you can see in the Client/DataStreamer.cpp`

### sendMessage

- send total size of the message (`4 bytes, int32`)
- send message content (`total size of the message`)

### receiveMessage

- receive total size of the incoming message (`4 bytes, int32`)
- receive message content (`total size of the message`)

### sendFileUsingChunks

- send total size of the data (`size of long long, 8 bytes`)
- while total sent less than total size:
  1.  send chunk size (`size of long long, 8 bytes`)
  2.  take size of chunk bytes data from the file and send it to the server

### receiveChunkedDataToFile

- receive integer (`4 bytes, int32`) from the server which is total size of the data
- while total received bytes is less than total size:
  1.  receive integer (`4 bytes, int32`) from the server which is chunk size
  2.  receive char array with size of chunk
  3.  append that data to file
