/*
*	Author	:	Stephen Amey
*	Date	:	June 10, 2019
*	Purpose	: 	This library is used to interface with a linear actuator with position feedback. 
*				It is specifically tailored to the Western University HAB project.
*/

//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "HAB_Actuator.h"


//--------------------------------------------------------------------------\
//								  Constructor					   			|
//--------------------------------------------------------------------------/


	HAB_Actuator::HAB_Actuator(const char* namePtr, uint8_t act_en, uint8_t act_push, uint8_t act_pull,
		uint8_t act_pos, uint8_t heat_en, uint8_t thermistor, double openAlt, double closeAlt
	){
		//Sets the variables
		strcpy(this->namePtr, namePtr);
		this->act_en = act_en;
		this->act_push = act_push;
		this->act_pull = act_pull;
		this->act_pos = act_pos;
		this->heat_en = heat_en;
		this->thermistor = thermistor;
		this->openAlt = openAlt;
		this->closeAlt = closeAlt;
		moveEnabled = false;
		heatEnabled = false;
		hasOpened = false;
		hasEnteredInterval = false;
		actuatorOverride = false;
		actuatorOverrideOpen = false;
		heaterOverride = false;
		heaterOverrideEnabled = false;
		
		//Sets the pins
		pinMode(heat_en,       OUTPUT);
		pinMode(act_en,        OUTPUT);
		pinMode(act_push,      OUTPUT);
		pinMode(act_pull,      OUTPUT);
		pinMode(thermistor,    INPUT);
		pinMode(act_pos,       INPUT);
	}
	
	
//--------------------------------------------------------------------------\
//								   Functions					   			|
//--------------------------------------------------------------------------/


	//--------------------------------------------------------------------------------\
	//Getters-------------------------------------------------------------------------|
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getName																	|
		|	Purpose: 	Gets the name of the actuator pod.										|
		|	Arguments:	void																	|
		|	Returns:	double																	|
		\*-------------------------------------------------------------------------------------*/
			char* HAB_Actuator::getName(){
				return namePtr;
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getOpenAlt																|
		|	Purpose: 	Gets the opening altitude of the actuator.								|
		|	Arguments:	void																	|
		|	Returns:	double																	|
		\*-------------------------------------------------------------------------------------*/
			double HAB_Actuator::getOpenAlt(){
				return openAlt;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getCloseAlt																|
		|	Purpose: 	Gets the closing altitude of the actuator.								|
		|	Arguments:	void																	|
		|	Returns:	double																	|
		\*-------------------------------------------------------------------------------------*/
			double HAB_Actuator::getCloseAlt(){
				return closeAlt;
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getPosition																|
		|	Purpose: 	Gets the current position of the actuator.								|
		|	Arguments:	void																	|
		|	Returns:	integer																	|
		\*-------------------------------------------------------------------------------------*/
			uint16_t HAB_Actuator::getPosition(){
				uint16_t reading = analogRead(act_pos);
				
				//pos += (moveEnabled ? (isMovingOpen ? -255 : 255) : 0);
				//return pos;
				
				return reading;
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isMoveEnabled															|
		|	Purpose: 	Gets the moveEnabled boolean variable value.							|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isMoveEnabled(){
				return moveEnabled;
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isHeatEnabled														|
		|	Purpose: 	Gets the heatEnabled boolean variable value.						|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isHeatEnabled(){
				return heatEnabled;
			}
	
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isClosed																|
		|	Purpose: 	Returns true if the pod is closed.										|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isClosed(){
				uint16_t reading = analogRead(act_pos);	

				//return (pos >= POD_CLOSED);
				return (reading >= POD_CLOSED);
			}
	
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isFullyOpen																|
		|	Purpose: 	Returns true if the pod is fully open.									|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isFullyOpen(){
				unsigned int reading = analogRead(act_pos); //Check trend at all?
				
				//return (pos <= POD_OPEN);
				return (reading <= POD_OPEN);
			}
						
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getTemperature															|
		|	Purpose: 	Returns the temperature read by the thermistor.							|
		|	Arguments:	void																	|
		|	Returns:	float																	|
		\*-------------------------------------------------------------------------------------*/
			float HAB_Actuator::getTemperature(){
				//Get reading
				float reading = analogRead(thermistor);
				reading = (1023 / reading)  - 1;     // (1023/ADC - 1) 
				reading = SERIESRESISTOR / reading;  // 10K / (1023/ADC - 1)

				//Get temperature
				float temp;
				temp = reading / THERMISTORNOMINAL;     	// (R/Ro)
				temp = log(temp);                  			// ln(R/Ro)
				temp /= BCOEFFICIENT;                   	// 1/B * ln(R/Ro)
				temp += 1.0 / (TEMPERATURENOMINAL + 273.15);// + (1/To)
				temp = 1.0 / temp;                 			// Invert
				temp -= 273.15;                         	// convert to C
				
				return temp;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getHasOpened															|
		|	Purpose: 	Returns true if the actuator has opened once already.					|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::getHasOpened(){
				return hasOpened;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isInInterval															|
		|	Purpose: 	Returns true if the provided altitude is within the actuator's inteval. |
		|	Arguments:	float																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isInInterval(float altitude){
				//If open < close
				if(openAlt < closeAlt){
					return (altitude >= openAlt && altitude < closeAlt);
				}
				//If open >= close
				else{
					//If we reached the open altitude 
					if(!hasEnteredInterval && altitude >= openAlt){
						hasEnteredInterval = true;
						return true;
					}
					//If we atleast reached the open altitude and are still above the closing altitude
					else if(hasEnteredInterval && altitude > closeAlt){
						return true;
					}
					//If not reached opening altitude yet, or below the closing altitude
					else{
						return false;
					}			
				}
			}
					
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isOpening																|
		|	Purpose: 	Returns true if the last direction of the actuator is retracting 		|
		|				(pod opening), else false if extending (pod closing).					|
		|	Arguments:	none																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isOpening(){
				return isMovingOpen;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isLocked																|
		|	Purpose: 	Returns true if the lock boolean is set to true.						|
		|	Arguments:	none																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isLocked(){
				return locked;
			}
			
	
	//--------------------------------------------------------------------------------\
	//Setters-------------------------------------------------------------------------|
	
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setMoveEnabled															|
		|	Purpose: 	Sets the moveEnabled boolean variable.									|
		|	Arguments:	boolean																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			//void HAB_Actuator::setMoveEnabled(boolean moveEnabled){
			//	digitalWrite(act_en, (moveEnabled ? HIGH : LOW));
			//	this->moveEnabled = moveEnabled;
			//}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setOpenAltitude															|
		|	Purpose: 	Sets the actuator's opening altitude.									|
		|	Arguments:	double																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::setOpenAltitude(double openAlt){
				this->openAlt = openAlt;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setCloseAltitude														|
		|	Purpose: 	Sets the actuator's closing altitude.									|
		|	Arguments:	double																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::setCloseAltitude(double closeAlt){
				this->closeAlt = closeAlt;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setHasOpened															|
		|	Purpose: 	Sets the hasOpened boolean if desired.									|
		|	Arguments:	bool																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::setHasOpened(bool hasOpened){
				this->hasOpened = hasOpened;
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setLock																	|
		|	Purpose: 	Sets the locked boolean.												|
		|	Arguments:	bool																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::setLock(bool locked){
				this->locked = locked;
			}

	//--------------------------------------------------------------------------------\
	//Miscellaneous-------------------------------------------------------------------|	
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		extend																	|
		|	Purpose: 	Extends the actuator to its maximum length.								|
		|	Arguments:	void																	|
		|	Returns:	boolean																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::extend(){
				//Enables the actuator
				digitalWrite(act_en, HIGH);
				this->moveEnabled = true;
				
				this->isMovingOpen = false;
				
				//Moves the actuator
				digitalWrite(act_push, HIGH);
				digitalWrite(act_pull, LOW);
				HAB_Logging::printLog("Started extending actuator of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" (Closing)", "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		retract																	|
		|	Purpose: 	Retracts the actuator to its minimum length.							|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::retract(){				
				//Enables the actuator
				digitalWrite(act_en, HIGH);
				this->moveEnabled = true;
				
				this->isMovingOpen = true;
				
				//Moves the actuator
				digitalWrite(act_push, LOW);
				digitalWrite(act_pull, HIGH);
				hasOpened = true; //Set upon retraction so that it does not reopen
				HAB_Logging::printLog("Started retracting actuator of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" (Opening)", "");
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		halt																	|
		|	Purpose: 	Halts movement of the actuator.											|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::halt(){
				//Disables the actuator
				digitalWrite(act_en, LOW);
				this->moveEnabled = false;
				
				//Halts the actuator
				digitalWrite(act_push, LOW);
				digitalWrite(act_pull, LOW);
				HAB_Logging::printLog("Halted actuator of ");
				HAB_Logging::printLogln(this->getName(), "");
			}	

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideActuatorHalt													|
		|	Purpose: 	Halts movement of the actuator, dsiables any overrides.					|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideActuatorHalt(){
				//Disables the actuator
				digitalWrite(act_en, LOW);
				this->moveEnabled = false;
				
				//Disables any overrides
				actuatorOverride = false;
				actuatorOverrideOpen = false;
				
				//Halts the actuator
				digitalWrite(act_push, LOW);
				digitalWrite(act_pull, LOW);
				HAB_Logging::printLog("Halted actuator of ");
				HAB_Logging::printLogln(this->getName(), "");
			}				
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideActuatorOpen													|
		|	Purpose: 	Overrides the actuator, commands it to open.							|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideActuatorOpen(){
				actuatorOverride = true;
				actuatorOverrideOpen = true;
				hasOpened = true;
				HAB_Logging::printLog("Actuator of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" overridden to OPEN state", "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideActuatorClose													|
		|	Purpose: 	Overrides the actuator, commands it to close.							|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideActuatorClose(){
				actuatorOverride = true;
				actuatorOverrideOpen = false;
				HAB_Logging::printLog("Actuator of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" overridden to CLOSED state", "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideActuatorRelease													|
		|	Purpose: 	Releases override of the actuator.										|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideActuatorRelease(){
				HAB_Actuator::halt();
				actuatorOverride = false;
				actuatorOverrideOpen = false;
				HAB_Logging::printLog("Actuator of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" override RELEASED", "");
				
				//Should possibly halt it here to disable moveEnabled
			}	
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isActuatorOverridden													|
		|	Purpose: 	Returns true if the actuator is overridden.								|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isActuatorOverridden(){
				return actuatorOverride;
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isActuatorOverrideOpen													|
		|	Purpose: 	Returns true if the the actuator override is open.						|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isActuatorOverrideOpen(){
				return actuatorOverrideOpen;
			}	
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		startHeating															|
		|	Purpose: 	Starts the heating of the actuator.										|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::startHeating(){
				digitalWrite(heat_en, HIGH);
				heatEnabled = true;
				HAB_Logging::printLog("Started heating ");
				HAB_Logging::printLogln(this->getName(), "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		stopHeating																|
		|	Purpose: 	Stops the heating of the actuator.										|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::stopHeating(){
				digitalWrite(heat_en, LOW);
				heatEnabled = false;
				HAB_Logging::printLog("Stopped heating ");
				HAB_Logging::printLogln(this->getName(), "");
			}	
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideHeaterEnable													|
		|	Purpose: 	Overrides the heater, commands it to turn on.							|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideHeaterEnable(){
				heaterOverride = true;
				heaterOverrideEnabled = true;
				HAB_Logging::printLog("Heater of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" overridden to ENABLED state", "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideHeaterDisable													|
		|	Purpose: 	Overrides the heater, commands it to turn off.							|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideHeaterDisable(){
				heaterOverride = true;
				heaterOverrideEnabled = false;
				HAB_Logging::printLog("Heater of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" overridden to DISABLED state", "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		overrideHeaterRelease													|
		|	Purpose: 	Releases override of the heater.										|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::overrideHeaterRelease(){
				heaterOverride = false; //Could do a check on if this is set beforehand, but not very important
				heaterOverrideEnabled = false;
				HAB_Logging::printLog("Heater of ");
				HAB_Logging::printLog(this->getName(), "");
				HAB_Logging::printLogln(" override RELEASED", "");
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isHeaterOverridden														|
		|	Purpose: 	Returns true if the heater is overridden.								|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isHeaterOverridden(){
				return heaterOverride;
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isHeaterOverrideOpen													|
		|	Purpose: 	Returns true if the the heater override is enabled.						|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_Actuator::isHeaterOverrideEnabled(){
				return heaterOverrideEnabled;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		deactivateAll															|
		|	Purpose: 	Deactivates the actuator and temperature.								|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_Actuator::deactivateAll(){
				//Disables the actuator
				digitalWrite(act_en, LOW);
				this->moveEnabled = false;
				
				//Halts the actuator
				digitalWrite(act_push, LOW);
				digitalWrite(act_pull, LOW);
				
				//Disables the heater
				digitalWrite(heat_en, LOW);
				heatEnabled = false;
				
				//Disable overrides
				actuatorOverride = false;
				actuatorOverrideOpen = false;
				heaterOverride = false;
				heaterOverrideEnabled = false;
				
				//Log the event
				HAB_Logging::printLog("Deactivated actuator and heater of ");
				HAB_Logging::printLogln(this->getName(), "");				
			}