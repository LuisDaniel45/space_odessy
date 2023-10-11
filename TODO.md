# TODO
   - [x] Research how to do the game loop efficiently.
        ```python 
            # it is pseudocode but this way it has color

           X = number of times to run update 
           Y = time between draw (ideally 60 times a second) 

           update() X times in Y time 

           if (update() finished X times run and Y time not complete)
               sleep until Y time finished
               draw screen 

           else if (update() not finished X times )
               break update execution 
               draw screen

        ```

   - [x] Make use of pixmaps 
   - [x] Make and render game skins 
   - [x] Make the Game resizable 
   - [x] Find font to use for start and game_over state
   - [x] Make start and Game Over look good
   - [x] Make Font resizable
   - [x] Introduce Sound 
   - [x] Debug collision 
   - [x] Make sound table 
   - [x] Make Windows Version
   - [x] Make keys remapable 
   - [x] Make config menu 
   - [x] Add a score board  
   - [ ] Refactor, test, and improve code 
   - [ ] Make music for game
   - [ ] Make a mute for music 
   - [ ] Make Render parallel for efficiency 
   - [x] Upload to Github
   - [ ] Make bundle to ship 


