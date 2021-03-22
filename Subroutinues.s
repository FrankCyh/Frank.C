.global _start
_start:
	
/*********************************/
/*****     Subroutinues     ******/
/*********************************/

/* Read from Switches and write to LEDR */
	LDR		R3, =0xFF200000
LOOP:
	LDR		R2, [R3, #0x40] // read SW
	STR		R2, [R3]		// write LEDR
	B		LOOP

/* Add list Byte */
// add the list of byte in the tag LIST, return the sum in R0
		MOV		R1, #LIST_byte		/* Point R1 to the start of the list */
		LDRB 	R2, [R1]		/* Initialize R2 with the first number */

		LDRB	R3, [R1, #1]	/* Get the next number */
		ADD		R2, R2, R3
		LDRB	R3, [R1, #2]	/* Get the next number */
		ADD		R2, R2, R3
		LDRB	R3, [R1, #3]	/* Get the next number */
		ADD		R2, R2, R3
		MOV		R0, R2
		
LIST_byte:
		.byte	10, 20, 30, 40				/* The numbers to be added */


/*** Add list Byte ***/
// add the list of byte in the tag LIST, return the sum in R0
  		MOV     R1, #LIST_word                   /* Point R1 to the start of the list */
  		LDR     R2, [R1]                    /* Initialize R2 with the first number */
		LDR     R3, [R1, #4]                /* Get the next number */
		ADD     R2, R2, R3
		LDR     R3, [R1, #8]                /* Get the next number */
		ADD     R2, R2, R3
		LDR     R3, [R1, #12]               /* Get the next number */
		ADD     R2, R2, R3
		MOV		R0, R2
		
LIST_word:
		.byte	10, 20, 30, 40				/* The numbers to be added */
		
		
/*** Count number of consecutive ones in a word ***/
// Count the number of consecutive 1's in a word, return the result in R0
 		MOV     R1, #TEST_NUM  		// load the data word ...
        LDR     R1, [R1]       	 	// into R1

        MOV     R0, #0         		// R0 will hold the result
LOOPONES:
		CMP     R1, #0         		// loop until the data contains no more 1's
        BEQ     ENDONES             
        LSR     R2, R1, #1    	 	// perform SHIFT, followed by AND
        AND     R1, R1, R2      
        ADD     R0, #1         		// count the string length so far
		B		LOOPONES
ENDONES:
		MOV     PC, LR 
		
		
/*** Count number of consecutive zeroes in a word ***/
// find the largest number of 0s in R1, return the result in R0
ZEROS:     	MOV     R1, #TEST_NUM   
			LDR 	R1, [R1]
			MOV		R0, #0			// R0 will hold the result
			MVN		R3, #0			// use move negative to initialize R3 of all 1's
LOOPZEROS: 	CMP		R1, R3			// loop until the data is the same from last time
          	BEQ     ENDZEROS   
			MOV		R3, R1
          	ASR     R2, R1, #1      // perform SHIFT, must shift the same sign, followed by OR
          	ORR     R1, R1, R2 
          	ADD     R0, #1          // count the string length so far
          	B       LOOPZEROS            
ENDZEROS:   SUB		R0, #1			// R8 will be equal to R1 one more clock after stable
			MOV     PC, LR 


/*** Count number of alternating 10's in a word ***/
// find the largest number of 10s in R1, return the result in R0
ALTERNATE: 	MOV     R1, #TEST_NUM   
			LDR 	R1, [R1]		// input in R1
LOOPALTERNATE:     	
			ASR     R2, R1, #1      // perform SHIFT, followed by XOR
          	EOR     R1, R1, R2      
			MOV     R0, #0          // R0 will hold the result
          	BL		LOOPONES
			ADD		R0, #1			// total consecutive 01's should be the number of consecutive 1s + 1
			MOV		PC, LR


/*** convert from a number to 7 segment code ***/
/* load the corresponding 8 bit 7-segment code of nunmber R0 into R0 */
SEG7_CODE:  MOV     R1, #BIT_CODES  
            ADD     R1, R0         // index into the BIT_CODES "array"
            LDRB    R0, [R1]       // load the bit pattern (to be returned)
            MOV     PC, LR              

BIT_CODES:  .byte   0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110
            .byte   0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01100111
            .skip   2      // pad with 2 bytes to maintain word alignment
			
		
			
/* Submodule: perform the integer division R0 / R1.
 * Returns: quotient in R1, and remainder in R0 */
MOD:     	MOV    	R2, #0
MOD_LOOP:
			CMP    	R0, R1
            SUBGE  	R0, R1
			ADDGE  	R2, #1
			BGE		MOD_LOOP

		    MOV    	R1, R2     // quotient in R1 (remainder in R0)
            MOV    	PC, LR
			
			
/* Submodule: calculate the largest number j (j^2 <= R0) */
// j is outputted using R0
ROOT:
			MOV		R1, #1
ROOT_LOOP:
			MUL 	R2, R1, R1
			CMP 	R2, R0
			ADDLT 	R1, #1
			BLT		ROOT_LOOP
			// if j^2 = R0, return j
			MOVEQ	R0, R1
			MOVEQ 	PC, LR	// return
			// if j^2 > R0, return j - 1
			SUBGT	R1, #1
			MOV		R0, R1 
			MOV 	PC, LR
			
			
/* Submodule: determine if number R0 is a prime number */
// return the result in R0, 0 for prime, 1 for not prime
PRIME_OR_NOT:
	PUSH	{R4, LR}
	MOV		R4, R0	// move the number to be checked in R4
	MOV		R0, #0 // The final answer is stored in R0
	
// special case for #2
	CMP 	R4, #2
	MOVEQ	R0, #1
	BEQ		PRIME_END
	
// even number are not prime number
	MOV		R1, #2 	// divisor = 2
	MOV		R0, R4
	BL		MOD
	CMP 	R0, #0	
	BEQ 	PRIME_END
	
// For odd number, find the largest divisor
	MOV		R0, R4
	BL		ROOT
	MOV		R3, R0 // use R3 to store the largest divisor in R0
	
CHECK:
	CMP		R3, #2 // have remainder for all divisor larger than 2
	MOVEQ	R0, #1 // prime
	BEQ		PRIME_END

	MOVGT	R0, R4
	MOVGT	R1, R3
	BL		MOD

	CMP		R0, #0
	BEQ		PRIME_END
	
	SUB		R3, #1	// loop through smaller divisor
	B		CHECK
	
PRIME_END:	
	POP		{R4, LR}


/* Subroutine to display HEX of a number (at most 9999) */
// Input in R0, output to HEX
DISPLAY_HEX:
			PUSH 	{R4, LR}
			MOV		R1, #100
			BL		DIVIDE
			MOV		R4, R1			// save the number to be displayed on HEX 3-2
									// R0 stores the remainder devided by 100
			
			/* Display HEX on HEX 1-0 */
			MOV		R1, #10
			BL      DIVIDE          // ones digit will be in R0; tens digit in R1
	       	MOV     R2, R1          // save the tens digit in R4
            BL      SEG7_CODE       // calculate the seven segment bit code of remainder
            MOV     R3, R0          // save seven segment display bit code in R3
            MOV     R0, R2          // retrieve the tens digit
            BL      SEG7_CODE       // calculate the seven segment bit code of the quotient
            LSL     R0, #8			// left shift the result 8 bits
            ORR     R3, R0			// use or to get the 16 bit seven segment code
			
			/* Display HEX on HEX 3-2 */
			MOV		R1, #10
			MOV		R0, R4			// Calculate HEX 3-2
			BL      DIVIDE          // ones digit will be in R0; tens digit in R1
	       	MOV     R4, R1          // save the tens digit in R4
            BL      SEG7_CODE       // calculate the seven segment bit code of remainder
            MOV     R2, R0          // save seven segment display bit code in R2
            MOV     R0, R4          // retrieve the tens digit
            BL      SEG7_CODE       // calculate the seven segment bit code of the quotient
            LSL     R0, #8			// left shift the result 8 bits
            ORR     R2, R0			// use or to get the 16 bit seven segment code
			LSL		R2, #16			// shift another 16 left for HEX 3-2
			ADD		R2, R3
			LDR		R4, =#0xFF200020// HEX address
			STR    	R2, [R4]
			POP 	{R4, PC}
			
			
/*** Submodule: achieve an approximate delay using delay loop ***/
	MOV 	R8, #0x80000 // 0.25 sec
DELAY:
	SUBS	R8, #1
	BNE		DELAY
			
			
			
/*** ARM A9 private timer ***/
// set up the Timer, placed in main function
INITIALIZE_TIMER:
        LDR     R8, =0xFFFEC600     // ARM A9 Private Timer address
        LDR     R2, =50000000       // timeout = 1/(200 MHz) x 5x10^7 = 0.25 sec
        STR     R2, [R8]            // write to timer load register
        MOV     R2, #0b011          // set bits: mode = 1 (auto), enable = 1
        STR     R2, [R8, #0x8]      // write to control register, to start timer
		
DELAY:  LDR     R0, [R8, #0xC]      // read timer status
        CMP     R0, #0
        BEQ     DELAY
        STR     R0, [R8, #0xC]      // reset timer flag bit to 1 to reset the F bit to 0


		MOV 	R8, #0x80000
DELAY:
		SUBS	R8, #1
		BNE		DELAY
		
	
/*** Polled I/O : Wait for key release ***/
		LDR     R1, [R2, #0x50]     // load KEYs, R2 stores I/O base address
		// .... Check for KEY values ...
		CMP		R1, #0b10		// for example, check for KEY1
		BEQ		KEY_1
		MOV		R4, R1				// use R4 to store the last switch value
		
KEY_1:
		// .... some implementation
		B		WAIT
		
WAIT:   LDR     R0, [R2, #0x50]     // poll the KEYs
        CMP     R0, R4
        BNE     WAIT 				// if not equal the key hasn't been returned, wait
		B		MAIN				// if returned















		
END:	B 		END


