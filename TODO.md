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
   - [ ] Find font to use for start and game_over state
   - [ ] Make start and Game Over look good
   - [x] Make the Game resizable 
   - [ ] Do Game Mechanichs
   - [ ] Introduce Sound 
   - [ ] Refactor, test, and improve code 
   - [ ] Upluad to Github
   - [ ] Make Windows Version
   - [ ] Make bundle to ship 


