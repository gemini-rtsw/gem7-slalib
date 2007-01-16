#include "slalib.h"
#include "slamac.h"
void slaI2opad ( IOpars *ioprms )
/*
**  - - - - - - - - - -
**   s l a I 2 o p a d
**  - - - - - - - - - -
**
**  In the CIRS to observed place star-independent parameter structure,
**  disable the diurnal aberration.
**
**  Returned:
**
**     ioprms IOpars*   star-independent CIRS-to-observed parameters:
**      along     double
**      phi       double
**      hm        double
**      xpl       double
**      ypl       double
**      sphi      double
**      cphi      double
**      diurab    double    set to zero
**      p         double
**      tk        double
**      rh        double
**      tlr       double
**      wl        double
**      refa      double
**      refb      double
**      eral      double
**
**  Notes:
**
**  1  This capability is provided to bypass the geocentric stage in the
**     transformation from ICRS to CIRS coordinates;  the site velocity
**     and position can be specified at that point.  One motivation for
**     doing so would be to include (stellar) geocentric parallax, which
**     is neglected in the slaI2o functions.
**
**  2  To re-instate the correction for diurnal aberration, use
**     slaI2opa.
**
**  3  For more information, see slaI2opa.
**
**  Last revision:   17 January 2006
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{

   ioprms->diurab = 0.0;

}
