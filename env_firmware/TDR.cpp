#include "TDR.h"
#include "global.h"

/*
 *  Function:       bool TDR::setup()
 *  Description:    setup the serial
 */
bool TDR::setup() {

  // turn the pin on to enable sensor
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);
  delay(8000);
    
	Wire.begin();
	// tdr counter for variables
	bool updated = false;
	Wire.requestFrom(8, 1);    // request 1 bytes from slave device #8
	while (Wire.available()) { // slave may send less than requested
		char c = Wire.read(); // receive a byte as character
		if (c == '\n')
		{
			updated = true;
		}
	}
  digitalWrite(PIN, LOW);
  //timer_tdr_read=TimerMillis();
  return updated;
} // end of setup()

/*
 *  Function:       void TDR::print_data()
 *  Description:    print all data
 */
void TDR::print_data() {

    print_debug(data,                               // data where we store values
                time_data,                          // where we store time data
                TDR_num_of_variables,               // number of sensor we use
                counter_col,                        // counter for coloumn index
                counter_row,                        // counter for row index
                counter_col_overflow);              // flag for coloumn counter overflow

} // end of print_data()

/*
 *  Function:    TDR::exec_timer()
 *  Description: execute read if the timer is right
 */
bool TDR::exec_timer() {

    if((millis() - exec_timer_last >= exec_time) & (exec_time_stage==1)) {
      //stage 1: initiate
      digitalWrite(EN_PIN, HIGH);
      exec_timer_last = millis();
      exec_time_stage=2;
      #ifdef debug
          serial_debug.println("TDR (exec_timer) - powerup");
      #endif
    }
    
    if((millis() - exec_timer_last >= exec_time_read) & (exec_time_stage==2)) {
        exec_time_stage=1;
        if(read()) {
            //print_data();
            #ifdef debug
                serial_debug.println("TDR (exec_timer) - executed read");
            #endif
            return true;
        }
        return false;
        
    }

    return false;
} // end of exec_timer()

/*
 *  Function:       bool TDR::read_allTDR()
 *  Description:    read all TDR data
 */
boolean TDR::read_allTDR() {
	Wire.requestFrom(8, 80);    // request 80 bytes from slave device #8
	int i = 0;
	int j = 0;
	float sgn = 1.0;
	bool newSensor = true;
	bool start = false;
	String buf = "";
#ifdef debug
	//serial_debug.print("Buffer: ");
#endif
	while (Wire.available()) { // slave may send less than requested
		char c = Wire.read(); // receive a byte as character
#ifdef debug
		//serial_debug.print(c);
#endif
		//Check for starting sequence
		if (!start)
		{
			if (c == '\n')
			{
				start = true;
			}
		}
		else
		{
			//Check for stop sequence
			if (c == '!') {
				start = false;
			}
			//If new sensor reading
			else if (newSensor)
			{
				buf += c;
				i = buf.toInt(); //Get sensor position
				buf = "";
				newSensor = false;
			}
			else
			{
				if (c == '\r') {
					// \r - ignore
				}
				//Check for end symbol
				else if (c == '\n') {
					//End of sensor read
					read_data[i][j] = sgn*buf.toFloat(); //Parse last data
					buf = "";
					newSensor = true;
					read_data[i][0] = j;
					j = 0;
				}
				//Check for new value
				else if (c == '+') {
					if (j > 0)
					{
						read_data[i][j] = sgn*buf.toFloat(); //Parse last data
					}
					j++;
					buf = "";
					sgn = 1.0; //Positive sign
				}
				else if (c == '-') {
					if (j > 0)
					{
						read_data[i][j] = sgn*buf.toFloat(); //Parse last data
					}
					j++;
					buf = "";
					sgn = -1.0; //Negative sign
				}
				//Store to new value
				else {
					buf += c;
				}
			}
		}
	}//End while
  digitalWrite(EN_PIN, LOW);
#ifdef debug
	serial_debug.println("");
#endif

	//Convert to values
	current_vol_w_content = (int)(read_data[0][1]*100);
	update_16bit(data,                          // our data array
		time_data,                      // our time data array
		id_vol_w_content,                // pressure ID
		counter_col,                    // coloumn counter
		counter_row,                    // row counter
		counter_col_overflow,           // check if coloumn has overflow 
		TDR_num_of_variables,       // number of variables
		current_vol_w_content,                  // the value
		exec_timer_last);               // the last time

	current_soil_temp = (int)(read_data[0][2]*100);
	update_16bit(data,                          // our data array
		time_data,                      // our time data array
		id_soil_temp,                // pressure ID
		counter_col,                    // coloumn counter
		counter_row,                    // row counter
		counter_col_overflow,           // check if coloumn has overflow 
		TDR_num_of_variables,       // number of variables
		current_soil_temp,                  // the value
		exec_timer_last);               // the last time

	current_soil_perm = (int)(read_data[0][3] * 100);
	update_16bit(data,                          // our data array
		time_data,                      // our time data array
		id_soil_perm,                // pressure ID
		counter_col,                    // coloumn counter
		counter_row,                    // row counter
		counter_col_overflow,           // check if coloumn has overflow 
		TDR_num_of_variables,       // number of variables
		current_soil_perm,                  // the value
		exec_timer_last);               // the last time

	current_soil_elec = (int)(read_data[0][4]);
	update_16bit(data,                          // our data array
		time_data,                      // our time data array
		id_soil_elec,                // pressure ID
		counter_col,                    // coloumn counter
		counter_row,                    // row counter
		counter_col_overflow,           // check if coloumn has overflow 
		TDR_num_of_variables,       // number of variables
		current_soil_elec,                  // the value
		exec_timer_last);               // the last time

	current_other_data = (int)(read_data[0][5]);
	update_16bit(data,                          // our data array
		time_data,                      // our time data array
		id_other_data,                // pressure ID
		counter_col,                    // coloumn counter
		counter_row,                    // row counter
		counter_col_overflow,           // check if coloumn has overflow 
		TDR_num_of_variables,       // number of variables
		current_other_data,                  // the value
		exec_timer_last);               // the last time

#ifdef debug
	serial_debug.print("current_vol_w_content - Voltage:");
	serial_debug.println(current_vol_w_content);
	serial_debug.print("current_soil_temp - Temp:");
	serial_debug.println(current_soil_temp);
	serial_debug.print("current_soil_perm - Perm:");
	serial_debug.println(current_soil_perm);
	serial_debug.print("current_soil_elec - Elec:");
	serial_debug.println(current_soil_elec);
	serial_debug.print("current_other_data - Data:");
	serial_debug.println(current_other_data);
#endif

	return true;

} // end of read_allTDR()

/*
 *  Function:       bool TDR::read()
 *  Description:    read the sensors
 */
bool TDR::read() {
    if( read_allTDR() == true ) {
        return true;
    } else {
        return false;
    }
} // end of read()
