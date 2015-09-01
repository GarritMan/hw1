Havent finshed all later functions, unfortunately run out of time.
This is what i still wouldve done though:
	
	1. Create an update status function that runs through all the running processes and checks what their statuses are and then up dates their stucts to say if they are stopped or completed etc..
	
	2. add to the fg/foreground function to check if processes have been completed first or are currently stopped.
	
	3. add a signal handler to back ground proccesses that recieve the SIGTTIN signal when they request input from terminal but dont have the terminal, the handler would stop the process and send a signal to say the process has stopped. (this may be happening already, but im not entirely sure)
	
	4. I wanted to find a way to remove completed processes and processes that failed (invalid commands that were launched with an '&' ) from the linked list so that space isnt taken up by junk.
