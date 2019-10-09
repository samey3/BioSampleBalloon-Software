/*
*	Author	:	Stephen Amey
*	Date	:	June 17, 2019
*	Purpose	: 	This library is used to interface with a camera to capture and store images. 
*				It is specifically tailored to the Western University HAB project.
*/

//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "HAB_Camera.h"
	
	
//--------------------------------------------------------------------------\
//								  Constructor					   			|
//--------------------------------------------------------------------------/


	HAB_Camera::HAB_Camera(uint8_t chipSelect, uint8_t cam_rxPin, uint8_t cam_txPin){
		this->chipSelect = chipSelect;
		
		//Sets up the camera connection
		cam = Adafruit_VC0706(new SoftwareSerial(cam_txPin, cam_rxPin)); //cam_tx to our rx, cam_rx to our tx
		
		//Check for the camera
		cameraFound = cam.begin();
		if(!cameraFound) {
			HAB_Logging::printLogln("Failed to find camera.");
		}
		
		//Check for the SD card
		sdFound = (HAB_Logging::getStatus() || SD.begin(chipSelect));
		if(!sdFound) {
			HAB_Logging::printLogln("Failed to find SD card.");
		}
		
		//Gets a reference to the logging stringPtr
		stringPtr = HAB_Logging::getStringPtr();
	}
	
	HAB_Camera::HAB_Camera(){
		this->chipSelect = 0;
		cam = Adafruit_VC0706(new SoftwareSerial(0, 0));
		cameraFound = false;
		sdFound = false;
		
		//Gets a reference to the logging stringPtr
		stringPtr = HAB_Logging::getStringPtr();
	}	
	
	
//--------------------------------------------------------------------------\
//								   Functions					   			|
//--------------------------------------------------------------------------/


	//--------------------------------------------------------------------------------\
	//Getters-------------------------------------------------------------------------|
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getInfo																	|
		|	Purpose: 	Returns the camera info													|
		|	Arguments:	void																	|
		|	Returns:	char*																	|
		\*-------------------------------------------------------------------------------------*/
			char* HAB_Camera::getInfo(char* stringPtr){
				//If no camera found
				if(!cameraFound){ 
					strcpy(stringPtr, "No camera found.");
				}	
				else{			
					strcpy(stringPtr, cam.getVersion());
					if(!stringPtr){ strcpy(stringPtr, "Failed to get version"); }
				}
				return stringPtr;
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getReadyStatus															|
		|	Purpose: 	Returns true if both the camera and sd card are found.					|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Camera::getReadyStatus(){
				return (cameraFound && sdFound);
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		bufferHasData															|
		|	Purpose: 	Returns true if there is data in the camera buffer.						|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Camera::getBufferStatus(){
				return(strcmp(fileName, "") != 0 && bytesLeft > 0);
			}

            
	//--------------------------------------------------------------------------------\
	//Setters-------------------------------------------------------------------------|
	

	//--------------------------------------------------------------------------------\
	//Miscellaneous-------------------------------------------------------------------|	
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		captureImage															|
		|	Purpose: 	Capture an image with the camera.										|
		|	Arguments:	image size (integer 0 to 2)												|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Camera::captureImage(const char* fileName, uint8_t size){	
				//Ensure there is a camera
				if(!getReadyStatus()){ HAB_Logging::printLogln("No camera or SD card found."); return; }				
				if(strcmp(this->fileName, "") != 0){ HAB_Logging::printLogln("Cannot capture image: must write or unbuffer last capture."); return; }

				//Modify the image name here
				strcpy(stringPtr, itoa(imgCount++, stringPtr, 10));
				strcat(stringPtr, "_"); strcat(stringPtr, fileName);
				
				//Set image size
				switch(size){
					//Large
					case 0:
						cam.setImageSize(VC0706_640x480);  break;
					//Medium
					case 1:
						cam.setImageSize(VC0706_320x240);  break;
					//Small
					case 2:
						cam.setImageSize(VC0706_160x120); break;
					default:
						HAB_Logging::printLogln("Invalid image size, no image was captured.");
						return; //Returns, does not take a picture
				}

				//Capture the image
				cam.setImageSize(VC0706_320x240);
				if (cam.takePicture()){
					//Sets the filename which will be used during the SD write
					strcpy(this->fileName, stringPtr);
						
					//Gets the frame length
					bytesLeft = cam.frameLength();		

					//Outputs a message
					HAB_Logging::printLog("Captured image '");
					HAB_Logging::printLog(stringPtr, "");
					HAB_Logging::printLog("' successfully! (", "");
					HAB_Logging::printLog(itoa(bytesLeft, stringPtr, 10), "");			
					HAB_Logging::printLogln(" bytes)", "");						
				}
				else{
					HAB_Logging::printLog("Failed to capture image '");	
					HAB_Logging::printLog(stringPtr, "");
					HAB_Logging::printLogln("'.", "");
				}
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		writeImage																|
		|	Purpose: 	Iteratively writes the image to a file on every call.					|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Camera::writeImage(){
				//If there is an image to write
				if(strcmp(fileName, "") != 0 && bytesLeft > 0){
					
					//Open the file
					File imgFile = SD.open(fileName, FILE_WRITE);
					
					//If the file opened
					if(imgFile){
						for(int i = 0; i != WRITES_PER_LOOP; i++){							
							//Reads in the next 32 or remaining bytes
							bytesToRead = min(32, bytesLeft);
							buffer = cam.readPicture(bytesToRead);
							bytesLeft -= bytesToRead;
							
							//Write the buffer to the image
							imgFile.write(buffer, bytesToRead);
							
							//If no bytes left, unset the fileName
							if(bytesLeft <= 0){
								HAB_Logging::printLog("Finished writing image '");
								HAB_Logging::printLog(fileName, "");
								HAB_Logging::printLogln("' to SD!", "");
								strcpy(fileName, "");
								break;
							}							
						}
					}
					
					//Close the file
					imgFile.close();
				}
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		emptyImageBuffer														|
		|	Purpose: 	Drops any image stored in the cameras buffer. Allow 250ms delay after	|
		|				calling this function.													|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/		
			void HAB_Camera::emptyImageBuffer(){
				if(!getReadyStatus()){ HAB_Logging::printLogln("No camera or SD card found."); return; }				
				if(strcmp(fileName, "") == 0){ HAB_Logging::printLogln("The camera buffer is already empty."); return; }
				
				//Attempt to empty the camera's buffer				
				if(cam.reset()){
					strcpy(fileName, "");
					bytesLeft = 0;
					HAB_Logging::printLogln("Successfully emptied the camera buffer.");
				}
				else{
					HAB_Logging::printLogln("Failed to empty the camera buffer.");
				}
			}