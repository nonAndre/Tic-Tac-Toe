# Tic-Tac-Toe

<blockquote>
  <p>Final project for my OS course @ UniVR — A.Y. 2023/2024. A simple documentation (written in Italian) can be found 
  <a href="https://github.com/nonAndre/Tic-Tac-Toe/blob/main/Relazione.pdf">here</a>
  </p>
</blockquote>
<h1>Description</h1>

<p>This game is realised in C and it uses linux systems calls,semaphores and shared memory</p>
<p></p>

<h1>Usage</h1>
<p>The game is pretty straight forward , is the classic tic-tac-toe we all know but there some kind of twists.
  <ul>
  <li>1. You can set a duration for the turns to make things more interesting </li>
    
  <li>2. You can also play against a bot. Be careful is not as easy as you think</li>
</ul>
</p>
<p>
  First of all you have to open 3 terminals ( 1 for server and 2 for the clients ).
  <p>You will start the server by giving the command ./TriServer "optional timeout in second" "symbol of the first player" "symbol of the second player"</p>

  <p align="center">./TriServer 10 X O</p>

  <p>Then for the remaining terminals you will give the command ./TriClient "YourName"</p>

   <p align="center">./TriClient pl1</p>

   <p align="center">./TriClient pl2</p>
</p>

<blockquote>
  <p>This is what you should see if you did everything coorectly 
  </p>
</blockquote>

<div align="center">
<img src="https://github.com/nonAndre/Tic-Tac-Toe/blob/main/videos/normalGame.gif" alt="Demo Gif"  style=" max-width: 100%; border: 1px solid #ccc; padding: 10px;">
</div>

<blockquote>You can also stop the game from the server</blockquote>

<div align="center">
<img src="https://github.com/nonAndre/Tic-Tac-Toe/blob/main/videos/gameEnding.gif" alt="Demo Gif"  style=" max-width: 100%; border: 1px solid #ccc; padding: 10px;">
</div>

<blockquote>You can also play against a bot</blockquote>

<div align="center">
<img src="https://github.com/nonAndre/Tic-Tac-Toe/blob/main/videos/autoPlay.gif" alt="Demo Gif"  style="max-width: 100%;border: 1px solid #ccc; padding: 10px;">
</div>
