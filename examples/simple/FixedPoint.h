#ifndef _FixedPoint_h
#define _FixedPoint_h

// Time to write: 25 minutes.
// TIme to debug: 30 minutes.

/** @file
 *  @brief Manages a fixed point number format.
 *
 *	 The fixed point format is 16.16.  Converts to and from a canonical packed format of 
 * <pre>s i i i i i i i   i i i i i i i i   i f f f f f f f   f f f f f f f f</pre>
 * where 's' is the sign, 'i' are the integer bits, and 'f' are the fractional bits.
 *
 *   Can also convert from and to native floating point and integer formats.  
 *
 *	 Performs some basic arithmetic on the numbers also.
 */
 
 /* UT_INCLUDE_PATH(../../include) */ 
 // UT_REPLACE_INCLUDE "garbage", "stdexcept" 
 // UT_REPLACE_TOKEN   "U(\w+\d+)", "uu_\1"

#include <cstdlib>
#include <string>
#include <garbage>
#include "another.h"

typedef unsigned char       UInt8;
typedef unsigned short 		UInt16;
typedef unsigned int 		UInt32;
typedef unsigned long long  UInt64;

typedef signed char         SInt8;
typedef signed short 	    SInt16;
typedef signed int 		    SInt32;
typedef signed long long    SInt64;

/** A Fixed Precision number.  Uses a native
 * 16.16 fixed format for speed, but converts 
 * to and from signed integers, floats, and
 * a 32-bit packed fixed point format. */
class AFixedPoint /*%startFixture*/
{
   /// The fixed-point value.
   SInt32 fValue;
   
   /***** UnitTest Setup and Teardown ***********
   %{
     	int *someSetupValue;
   %}
   
   %setup
   %{
	  	someSetupValue = new int;
		*someSetupValue = 0;  
   %}
   
   %teardown
   %{
	 	delete someSetupValue;	   
   %}      
   **********************************************/
      
public:
   AFixedPoint():fValue(0)
   {
      // deliberately empty  
   }  
   
   ~AFixedPoint()
   {
      // deliberately empty          
   }     
   
   /// Converts an integer to fixed point.
   void Set(SInt32 value)
   {
      fValue = value<<16;      
   }
   
   /*%test(Set_SInt32)
     %{
	
		AFixedPoint fp_num;
		
		fp_num.Set(-1.0f);
		
		UT_ASSERT_EQUALS(fp_num.ToString(), "-1.00000");
				
	 %} */
   
   /// Converts a floating point value to fixed point
   void Set(float value)
   {
	  if (value>65536) throw std::range_error("Float value out of range for 16.16 fixed point.");
	   
      fValue = static_cast<UInt32>(value*65536.0 + (value > 0 ? 0.5 : -0.5));  
   }
   
   /*%test(Set_Float)
	 %{
		AFixedPoint fp_num;
	
		UT_ASSERT_NORESULT_EXCEPTION(fp_num.Set(1000000.0f));
		UT_EXPECT_EXCEPTION(fp_num.Set(1000000.0f), std::bad_alloc);
		
	 %} */
   
   void Set(const AFixedPoint &f)
   {
      fValue = f.fValue;
   }
   
   /// Converts from packed fixed format to fixed point
   void SetPacked(UInt32 value)
   {
      fValue = 0;
                        
      // Get integer section
      fValue |= (value & 0x7fff8000) << 1;
      
      // Get fractional component
      fValue |= (value & 0x00007fff) << 1;
      
      // Make it negative if necessary
      if (value & 0x80000000) fValue=-fValue;               
   }
   
   /// Converts from fixed point to floating point
   void Get(float &value)
   {
      value = fValue;
      value /= 65536.0;  
   }
   
   /// Converts from fixed point to a signed 32-bit integer
   void Get(SInt32 &value)
   {
      value = fValue>>16;
   }
   
   void GetPacked(UInt32 &value)
   {
      UInt32 packed=0;
      
      // Store the sign      
      if (fValue<0) packed |= (0x80000000);
      
      // Store the integer section
      packed |= (std::abs(fValue) & 0xffff0000)>>1; 
      
      // Store the fractional component
      packed |= (fValue & 0x0000ffff) >> 1;
      
      // Return it
      value = packed;
   }
   
   /** Converts the fixed point into a string of the form
    * <<sign>><<integer-portion>>.<<fractional portion>> */
   std::string ToString()
   {          
      char temp[33];
      float value;
      
      // Convert to float
      this->Get(value);
      
      // Use library to convert it.
      std::snprintf(temp, sizeof(temp), "%f", value);
      
      // Turn it into a string.
      return std::string(temp);
   }
   
   /** Dumps the value to a string for diagnostic purposes. */
   std::string DumpString()
   {
      char temp[33];
      UInt32 packed;
      
      // Convert to packed format
      this->GetPacked(packed);
            
      // Use library to convert it.
      std::snprintf(temp, sizeof(temp), "0x%x", packed);
      
      // Turn it into a string.
      return std::string(temp);
   }
   
   
   /** Addition operator. */
   AFixedPoint operator+(const AFixedPoint &n)
   {
      AFixedPoint result;
      
      result.fValue = fValue + n.fValue;
      
      return result;   
   }
   
   /** Subtraction operator. */
   AFixedPoint operator-(const AFixedPoint &n)
   {
      AFixedPoint result;
      
      result.fValue = fValue - n.fValue;
      
      return result;   
   }
   
   /** Multiplication operator. */
   AFixedPoint operator*(const AFixedPoint &n)
   {
      AFixedPoint res;
      
      SInt64 temp1 = fValue;
      SInt64 temp2 = n.fValue;
      SInt64 result = (temp1 * temp2)>>16;
      
      res.fValue = static_cast<SInt32>(result);
      
      return res;   
   }
   
   /** Division operator. */
   AFixedPoint operator/(const AFixedPoint &n)
   {
      AFixedPoint res;
      SInt64 temp2 = fValue;
      SInt64 temp1 = n.fValue;
      SInt64 result = (temp1<<16) / temp2;
      
      res.fValue = static_cast<SInt32>(result);
      
      return res;   
   }

}; /*%endFixture*/   


#endif
