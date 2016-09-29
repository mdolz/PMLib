/*
 * ArduPower.hpp
 *
 * Created on: 27/05/2016
 *
 * =========================================================================
 *  Copyright (C) 2016-, Manuel F. Dolz (maneldz@gmail.com)
 *
 *  This file is part of PMLib.
 *
 *  PMLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PMLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this PMLib.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =========================================================================
*/

#ifndef APCAPE_HPP
#define APCAPE_HPP

#include <chrono>
#include <iostream>
#include <memory>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace std::chrono;

class INA219
{
  private:
      // ===========================================================================
      //   I2C ADDRESS/BITS
      // ==========================================================================
      const unsigned short __INA219_ADDRESS                         = 0x40;    // 1000000 (A0+A1=GND)
      const unsigned short __INA219_READ                            = 0x01;
      // ===========================================================================
    
      // ===========================================================================
      //    CONFIG REGISTER (R/W)
      // ===========================================================================
      const unsigned short __INA219_REG_CONFIG                      = 0x00;
      // ===========================================================================
      const unsigned short __INA219_CONFIG_RESET                    = 0x8000;  // Reset Bit
      const unsigned short __INA219_CONFIG_BVOLTAGERANGE_MASK       = 0x2000;  // Bus Voltage Range Mask
      const unsigned short __INA219_CONFIG_BVOLTAGERANGE_16V        = 0x0000;  // 0-16V Range
      const unsigned short __INA219_CONFIG_BVOLTAGERANGE_32V        = 0x2000;  // 0-32V Range
    
      const unsigned short __INA219_CONFIG_GAIN_MASK                = 0x1800;  // Gain Mask
      const unsigned short __INA219_CONFIG_GAIN_1_40MV              = 0x0000;  // Gain 1, 40mV Range
      const unsigned short __INA219_CONFIG_GAIN_2_80MV              = 0x0800;  // Gain 2, 80mV Range
      const unsigned short __INA219_CONFIG_GAIN_4_160MV             = 0x1000;  // Gain 4, 160mV Range
      const unsigned short __INA219_CONFIG_GAIN_8_320MV             = 0x1800;  // Gain 8, 320mV Range
    
      const unsigned short __INA219_CONFIG_BADCRES_MASK             = 0x0780;  // Bus ADC Resolution Mask
      const unsigned short __INA219_CONFIG_BADCRES_9BIT             = 0x0080;  // 9-bit bus res = 0..511
      const unsigned short __INA219_CONFIG_BADCRES_10BIT            = 0x0100;  // 10-bit bus res = 0..1023
      const unsigned short __INA219_CONFIG_BADCRES_11BIT            = 0x0200;  // 11-bit bus res = 0..2047
      const unsigned short __INA219_CONFIG_BADCRES_12BIT            = 0x0400;  // 12-bit bus res = 0..4097
    
      const unsigned short __INA219_CONFIG_SADCRES_MASK             = 0x0078;  // Shunt ADC Resolution and Averaging Mask
      const unsigned short __INA219_CONFIG_SADCRES_9BIT_1S_84US     = 0x0000;  // 1 x 9-bit shunt sample
      const unsigned short __INA219_CONFIG_SADCRES_10BIT_1S_148US   = 0x0008;  // 1 x 10-bit shunt sample
      const unsigned short __INA219_CONFIG_SADCRES_11BIT_1S_276US   = 0x0010;  // 1 x 11-bit shunt sample
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_1S_532US   = 0x0018;  // 1 x 12-bit shunt sample
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_2S_1060US  = 0x0048;  // 2 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_4S_2130US  = 0x0050;  // 4 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_8S_4260US  = 0x0058;  // 8 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_16S_8510US = 0x0060;  // 16 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_32S_17MS   = 0x0068;  // 32 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_64S_34MS   = 0x0070;  // 64 x 12-bit shunt samples averaged together
      const unsigned short __INA219_CONFIG_SADCRES_12BIT_128S_69MS  = 0x0078;  // 128 x 12-bit shunt samples averaged together
    
      const unsigned short __INA219_CONFIG_MODE_MASK                = 0x0007;  // Operating Mode Mask
      const unsigned short __INA219_CONFIG_MODE_POWERDOWN           = 0x0000;
      const unsigned short __INA219_CONFIG_MODE_SVOLT_TRIGGERED     = 0x0001;
      const unsigned short __INA219_CONFIG_MODE_BVOLT_TRIGGERED     = 0x0002;
      const unsigned short __INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED = 0x0003;
      const unsigned short __INA219_CONFIG_MODE_ADCOFF              = 0x0004;
      const unsigned short __INA219_CONFIG_MODE_SVOLT_CONTINUOUS    = 0x0005;
      const unsigned short __INA219_CONFIG_MODE_BVOLT_CONTINUOUS    = 0x0006;
      const unsigned short __INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0x0007;
      // ===========================================================================
    
      // ===========================================================================
      //   SHUNT VOLTAGE REGISTER (R)
      // ===========================================================================
      const unsigned char __INA219_REG_SHUNTVOLTAGE                = 0x01;
      // ===========================================================================
    
      // ===========================================================================
      //   BUS VOLTAGE REGISTER (R)
      // ===========================================================================
      const unsigned char __INA219_REG_BUSVOLTAGE                  = 0x02;
      // ===========================================================================
    
      // ===========================================================================
      //   POWER REGISTER (R)
      // ===========================================================================
      const unsigned char __INA219_REG_POWER                       = 0x03;
      // ===========================================================================
    
      // ==========================================================================
      //    CURRENT REGISTER (R)
      // ===========================================================================
      const unsigned char __INA219_REG_CURRENT                     = 0x04;
      // ===========================================================================
    
      // ===========================================================================
      //    CALIBRATION REGISTER (R/W)
      // ===========================================================================
      const unsigned char __INA219_REG_CALIBRATION                 = 0x05;
      // ===========================================================================
      
      unsigned short address;
      int fd;
      float currentDivider_mA;
      float powerDivider_mW;

      int twosToInt( int val, int len )
      {
        if( val & ( 1 << len - 1 ))
          val = val - ( 1 << len );

	return val;
      }
    
  public: 
      // Constructor.
      INA219( std::string devId, unsigned short _address ) : address{ _address }, currentDivider_mA{ 1 }, powerDivider_mW{ 1 }
      {
        if(( fd = open( devId.c_str(), O_RDWR )) < 0 )
          std::cout << "Error opening device" << std::endl;

        if(( ioctl( fd, I2C_SLAVE, address)) < 0 )
          std::cout << "Error binding device" << std::endl;

        setCalibration_32V_4_5A( );	      
      };

      ~INA219( void )
      {
        std::cout << "Closing device" << std::endl;
        if(( close( fd )) < 0 )
          std::cout << "Error closing device" << std::endl;
      }

      void setCalibration_32V_4_5A( void )
      {
        currentDivider_mA = 7.142857;
        powerDivider_mW = 0.357142;

	unsigned char bytes[ 2 ] = { (0x16DB >> 8 ) & 0xFF, 0x16DB & 0xFF };

	i2c_smbus_write_i2c_block_data( fd, __INA219_REG_CALIBRATION, 2, (unsigned char *)bytes );
	
	unsigned short config = __INA219_CONFIG_BVOLTAGERANGE_32V      | 
                                __INA219_CONFIG_GAIN_8_320MV           | 
                                __INA219_CONFIG_BADCRES_12BIT          | 
                                __INA219_CONFIG_SADCRES_12BIT_1S_532US | 
                                __INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	bytes[0] = (config >> 8 ) & 0xFF;
	bytes[1] = config & 0xFF;

	i2c_smbus_write_i2c_block_data( fd, __INA219_REG_CONFIG, 2, (unsigned char*)bytes );
      }

      unsigned short getBusVoltage_raw( void )
      {
	unsigned char bytes[ 2 ] = { 0, 0 };
	unsigned short testint, othernew;
	unsigned int result = i2c_smbus_read_i2c_block_data( fd, __INA219_REG_BUSVOLTAGE, 2, (unsigned char*)bytes ); 

	if( result == -1 )
	{
	  return 0;
	}
	else
	{
          if( bytes[0] >> 7 == 1 )
	  {
            testint = ( bytes[0] * 256 + bytes[1] );
            othernew = twosToInt( testint, 16 );
	    return othernew >> 3 * 4;
	  }
          else
          {
            return (unsigned short)( ( ( (bytes[0] << 8) | bytes[1] ) >> 3 ) * 4 );
          }
	}
      }

      unsigned short getShuntVoltage_raw( void )
      {
	unsigned char bytes[ 2 ] = { 0, 0 };
	unsigned short testint, othernew;
	unsigned int result = i2c_smbus_read_i2c_block_data( fd, __INA219_REG_SHUNTVOLTAGE, 2, (unsigned char*)bytes ); 

	if( result == -1 )
	{
	  return 0;
	}
	else
	{
          if( bytes[0] >> 7 == 1 )
	  {
            testint = ( bytes[0] * 256 + bytes[1] );
            othernew = twosToInt( testint, 16 );
	    return othernew;
	  }
          else
          {
            return (bytes[0] << 8) | (bytes[1]);
          }
	}
      }


      unsigned short getCurrent_raw( void )
      {
	unsigned char bytes[ 2 ] = { 0, 0 };
	unsigned short testint, othernew;
	unsigned int result = i2c_smbus_read_i2c_block_data( fd, __INA219_REG_CURRENT, 2, (unsigned char*)bytes ); 

	if( result == -1 )
	{
	  return 0;
	}
	else
	{
          if( bytes[0]  >> 7 == 1 )
	  {
            testint = ( bytes[0] * 256 + bytes[1] );
            othernew = twosToInt( testint, 16 );
	    return othernew;
	  }
          else
          {
            return (bytes[0] << 8) | (bytes[1]);
          }
	}
      }

      unsigned short getPower_raw( void )
      {
	unsigned char bytes[ 2 ] = { 0, 0 };
	unsigned int testint, othernew;
	unsigned int result = i2c_smbus_read_i2c_block_data( fd, __INA219_REG_POWER, 2, (unsigned char*)bytes ); 
	//std::cout << "SMBUS returned: " << result << std::endl;

	if( result == -1 )
	{
	  return 0;
	}
	else
	{
          if( bytes[0]  >> 7 == 1 )
	  {
            testint = ( bytes[0] * 256 + bytes[1] );
            othernew = twosToInt( testint, 16 );
	    return othernew;
	  }
          else
          {
	    //std::cout << "SMBUS read: " << bytes[0] << ", " << bytes[1] << std::endl;
            return (bytes[0] << 8) | (bytes[1]);
          }
	}

      }

      float getShuntVoltage_mV( void )
      {
        float value = (float)getShuntVoltage_raw();
	return value * 0.001;
      }

      float getBusVoltage_V( void )
      {
        float value = (float)getBusVoltage_raw();
	return value * 0.001;
      }

      float getCurrent_mA( void )
      {
        float valueDec = (float)getCurrent_raw();
	valueDec /= currentDivider_mA;
	return valueDec;
      }

      float getPower_mW( void )
      {
        float valueDec = (float)getPower_raw();
	//std::cout << "SMBUS read: " << valueDec << std::endl;
	valueDec /= powerDivider_mW;
	return valueDec;
      }
};


namespace PMLib
{
    template <int n_lines = 8, int max_freq = 1200, bool pdu = false>
    class APCape : public Device {
      public:
        APCape(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {

            int addr[8] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };

            std::vector<std::unique_ptr<INA219>> devs;

            for( auto i = 0; i < n_lines; i++ )
              devs.emplace_back( new INA219( "/dev/i2c-2", addr[i] ) );

            while ( is_running() ){ 
                auto t1 = system_clock::now();

                for( auto i = 0; i < n_lines; i++ )
                  sample[i] = devs[i]->getPower_mW();

                yield( sample );

                auto t2 = system_clock::now();

                auto dms = duration_cast<microseconds>(t2 - t1);
                //std::cout << "Measurement took " << dms.count() << " microseconds" << std::endl;

		if( dms.count() < (int)(1e6/max_freq) )
                {
                  //std::cout << "Will sleep: " << (int)(1e6/max_freq) - dms.count() << " microseconds" << std::endl;
                  this_thread::sleep_for(chrono::microseconds((int)(1e6/max_freq) - dms.count()));
                }
            }

        } ) {};   
    };

    static RegisterDevice< APCape< 1 > > Reg_APCape1L("APCape-1L");
    static RegisterDevice< APCape< 2 > > Reg_APCape2L("APCape-2L");
    static RegisterDevice< APCape< 3 > > Reg_APCape3L("APCape-3L");
    static RegisterDevice< APCape< 4 > > Reg_APCape4L("APCape-4L");
    static RegisterDevice< APCape< 5 > > Reg_APCape5L("APCape-5L");
    static RegisterDevice< APCape< 6 > > Reg_APCape6L("APCape-6L");
    static RegisterDevice< APCape< 7 > > Reg_APCape7L("APCape-7L");
    static RegisterDevice< APCape< 8 > > Reg_APCape8L("APCape-8L");
}

#endif
