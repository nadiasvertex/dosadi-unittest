#ifndef h_FixedPoint
#define h_FixedPoint

// Time: 100 minutes.

/** @file
 *  @brief Manages a fixed point number format.
 *
 *  The fixed point format is 16.16.  Converts to and from a canonical packed 
 * format of 
 * <pre>
 *  s i i i i i i i   i i i i i i i i   i f f f f f f f   f f f f f f f f
 * </pre> 
 * where 's' is the sign, 'i' are the integer bits, and 'f' are the fractional 
 * bits. Can also convert from and to native floating point and integer 
 * formats.  
 */
 
/* ******** Unit Test Setup ****** 
 UT_INCLUDE_PATH(../../include)
 */

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

typedef unsigned char       UInt8;
typedef unsigned short      UInt16;
typedef unsigned int        UInt32;
typedef unsigned long long  UInt64;

typedef signed char         SInt8;
typedef signed short        SInt16;
typedef signed int          SInt32;
typedef signed long long    SInt64;

typedef float					 Float32;

/** A Fixed Precision number.  Uses a native
 * 16.16 fixed format for speed, but converts 
 * to and from signed integers, floats, and
 * a 32-bit packed fixed point format. */
class AFixedPoint /*%startFixture*/
{      
public:
	/** Construct an empty fixed point object. */
   AFixedPoint():fValue(0)
   {
      // deliberately empty  
   }
   
   /** Destroy the fixed point object. */
   ~AFixedPoint()
   {
      // deliberately empty          
   }

   /** Converts an integer to fixed point.
    * @param The signed integer value to assign to this object. 
    * @return true if the assignment suceeded, false if the value was too 
    * large. */
   bool Set(SInt32 value)
   {
      // Record the sign of the number
      bool negative = value < 0;
      
      // Get the absolute value.
      UInt32 abs_value = static_cast<UInt32>(std::abs(value));
      
      // Make sure that we don't overflow
      if (abs_value >= 1<<kRadixShift)
      {
	       return false;      
      }
      
      // Adjust it for the internal format
      fValue = static_cast<SInt32>(abs_value<<kRadixShift);    
      
      // Fix the sign.
      if (negative) fValue = -fValue;
      
      return true;
   }
   
   /*
   %test(SetSInt32_GetSInt32)
   %{
	   SInt32 test_int[4]  = { 1, -1, 2, -2 };
	   const char *ideal[4] = { "0x00008000", "0x80008000",
	   								 "0x00010000", "0x80010000" };
	   
	   for(UInt32 i=0; i<4; ++i)
   	{
	   	AFixedPoint fp_num;
	   	std::ostringstream msg;
	   	
      	bool result = fp_num.Set(test_int[i]);
      
      	msg << "The conversion from signed integer '" << test_int[i]
	      	 << "' should succeed.";
	      	 
	      UT_ASSERT_MSG(result, msg.str());
	      
	      // Pull the value back out and make sure it's right.	      
	      SInt32 tmp;	      
	      fp_num.Get(tmp);
	      							
	      UT_ASSERT_EQUALS_MSG(test_int[i], tmp, 
	      						 "The result should equal the ideal.");	      						 
	      
	      msg.str("");
	      msg  << "The canonical format should match the ideal: " 
	      	  << "ideal: " << ideal[i]
	      	  << " canonical: " << fp_num.DumpString();
	      
	      UT_ASSERT_EQUALS_MSG(fp_num.DumpString(), ideal[i], msg.str());
       
   	}   
   	
   	AFixedPoint fp_num;
   	
   	UT_ASSERT_FALSE_MSG(fp_num.Set(65536),
   								"The Set should fail because of range issues.");	   
	%}
	*/
   
   /** Converts a floating point value to fixed point
    * @param value The floating point value to assign to this object. 
    * @return true on sucess, false if the value is too large or is a NaN.*/
   bool Set(Float32 value)
   {
	   if (std::isnan(value))
	   {
		    return false;
	   }
	   
	   // Record the sign
	   bool negative = value < 0;
		
	   // Get the absolute value
	   double abs_value = std::fabs(value);	   
	   
	   // Check to see if it's too large.
	   if (static_cast<UInt32>(std::floor(abs_value)) >= 1<<kRadixShift) 
	   {
	   	return false;
   	}
	   
	   //  Scale the floating point so that we can move the radix to the right
	   //  place for our 16.16 format.
	   fValue = static_cast<UInt32>(abs_value * kRadixScaler);
	   
	   // Fix the sign.
      if (negative) fValue = -fValue;
      
      return true;	   	         
   }
   
   /* 
   %test(SetFloat32_GetFloat32)
   %{
	   float test_float[4]  = { 1.0f, -1.0f, 2.0f, -2.5f };
	   const char *ideal[4] = { "0x00008000", "0x80008000",
	   								 "0x00010000", "0x80014000" };
			   
		for(UInt32 i=0; i<4; ++i)
   	{
	   	AFixedPoint fp_num;
	   	std::ostringstream msg;
	   	
	      bool result = fp_num.Set(test_float[i]);
	     	
	      msg << "The conversion from '" 
	      	 << std::showpoint << test_float[i]
	      	 << "' should succeed.";
	      	 
	      UT_ASSERT_MSG(result, msg.str());
	      			
	      // Pull the value back out and make sure it's right.				
	      float tmp;	      
	      fp_num.Get(tmp);
	      							
	      UT_ASSERT_FLOAT_NEAR_MSG(test_float[i], tmp, 0.09f,
	      						 "The result should be within 0.09 of the ideal.");
	      						 
	      
	      msg.str("");
	      msg  << "The canonical format should match the ideal: " 
	      	  << "ideal: " << ideal[i]
	      	  << " canonical: " << fp_num.DumpString();
	      
	      UT_ASSERT_EQUALS_MSG(fp_num.DumpString(), ideal[i], msg.str());
	      						 
   	}  	
   	
   	AFixedPoint fp_num;
   	
   	UT_ASSERT_FALSE_MSG(fp_num.Set(65536.0f),
   								"The Set should fail because of range issues.");
	%}
	*/
      
   
   /** Assigns the value of 'f' to this fixed
    * point object. 
    * @param f The fixed point value to assign to this object. */   
   void Set(const AFixedPoint &f)
   {
      fValue = f.fValue;
   }
   
   /** Converts from packed fixed format to fixed point
    * @param value The packed unsigned 32-bit value. */
   void SetPacked(UInt32 value)
   {
	   UInt32 tmp=0;
	                           
      // Get integer section
      tmp |= (value & 0x7fff8000) << 1;
      
      // Get fractional component
      tmp |= (value & 0x00007fff) << 1;
      
      fValue = static_cast<SInt32>(tmp);
      
      // Make it negative if necessary
      if (value & 0x80000000) fValue=-fValue;               
   }
   
   /* 
   %test(SetPacked_GetPacked)
   %{
	   UInt32 test_packed[4] = { 0x8000, 0x80008000, 0x10000, 0x80014000 };
	   const char *ideal[4] = { "0x00008000", "0x80008000",
	   								 "0x00010000", "0x80014000" };
			   
		for(UInt32 i=0; i<4; ++i)
   	{
	   	AFixedPoint fp_num;
	   	std::ostringstream msg;
	   	
	      fp_num.SetPacked(test_packed[i]);
	      
	      // Pull the value back out and make sure it's right.
	      UInt32 tmp;
	      fp_num.GetPacked(tmp);
	      
	      UT_ASSERT_EQUALS_MSG(tmp, test_packed[i],
	      							"The packed value should equal the ideal.");	      
	     		      	            
	      msg.str("");
	      msg  << "The canonical format should match the ideal: " 
	      	  << "ideal: " << ideal[i]
	      	  << " canonical: " << fp_num.DumpString();
	      
	      UT_ASSERT_EQUALS_MSG(fp_num.DumpString(), ideal[i], msg.str());
	      						 
   	}  	   	
   	
	%}
	*/
   
   /** Converts from fixed point to floating point
    * @param A reference to a float wherein the result is stored.  */
   void Get(Float32 &value)
   {
	   // Record the sign of the number
      bool negative = fValue < 0;
      	   
      // It's more portable to work with positive numbers.
      value = std::abs(fValue);
            
      // Scale the value back down.
      value /= kRadixScaler;  
      
      // Fix the sign.
      if (negative) value = -value;
   }
   
   /** Converts from fixed point to a signed 32-bit integer
    * @param value A reference to a signed 32-bit integer wherein the result
    * is stored. */
   void Get(SInt32 &value)
   {
	   // Record the sign of the number
      bool negative = fValue < 0;
      	   
      // It's more portable to work with positive numbers.
      UInt32 tmp = std::abs(fValue);
	   
      // Kill the fractional part
      value = tmp>>kRadixShift;
      
      // Fix the sign.
      if (negative) value = -value;            
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
      float value;
      
      // Convert to float
      this->Get(value);
    	
      std::ostringstream out;      
      out << std::fixed << std::showpoint 
      	 //<< std::setprecision(6) 
      	 << value;  
      
      // Turn it into a string.
      return out.str();
   }
   
   /*
   %test(ToString)
   %{
	   AFixedPoint fp_num;
	   std::ostringstream msg;
	   
	   fp_num.Set(-1.0f);
	   std::string ts = fp_num.ToString();
	   
	   msg << "The idiomatic string conversion should look like:"
	   	 << " '-1.000000' but instead, looks like: '"
	   	 << ts << "'";
	   
	   UT_ASSERT_EQUALS_MSG(ts, "-1.000000", msg.str());
   %}
   */
   
   /** Dumps the value to a string for diagnostic purposes. */
   std::string DumpString()
   {	   
	   //  A 32-bit value, when converted to base 2, will occupy only 
	   // 32 digits.  Since we are converting to base 16, we can be
	   // guaranteed that it will occupy no more than 8 bytes, plus
	   // two for the base signifier.
      char temp[11];      
      UInt32 packed;
      
      // Convert to packed format
      this->GetPacked(packed);            
      
      // Safe conversion.
      std::snprintf(temp, sizeof(temp), "0x%08x", packed);
      
      // Turn it into a string.
      return std::string(temp);
   } 
   
private:
   /// The fixed-point value.
   SInt32 fValue;
   
   /** Given that this class uses a 16.16 
    * internal format to represent a 32-bit fixed
    * point number with 16 bits of integer and 16 bits
    * of fraction, use this constant to shift the bits
    * around the radix point in various operations. */
   static const UInt32 kRadixShift = 16;
   
   /** Provides the same information as kRadixShift,
    * except for floating point formats. Think of this
    * as a scaling value that gets the radix point to
    * the proper place for binary numbers, much like multiplying
    * by powers of ten will move the radix point for
    * decimal numbers. */
   static const Float32 kRadixScaler = 65536.0f;      
      
}; /*%endFixture*/

#endif
