#include "slalib.h"
#include "slamac.h"
void slaPlanel ( double date, int jform, double epoch, double orbinc,
                 double anode, double perih, double aorq, double e,
                 double aorl, double dm, double pv[6], int* jstat )
/*
**  - - - - - - - - - -
**   s l a P l a n e l
**  - - - - - - - - - -
**
**  Heliocentric position and velocity of a planet, asteroid
**  or comet, starting from orbital elements.
**
**  Given:
**     date    double     date, Modified Julian Date (JD - 2400000.5)
**     jform   int        choice of element set (1-3, see Note 3, below)
**     epoch   double     epoch of elements (TT MJD)
**     orbinc  double     inclination (radians)
**     anode   double     longitude of the ascending node (radians)
**     perih   double     longitude or argument of perihelion (radians)
**     aorq    double     mean distance or perihelion distance (AU)
**     e       double     eccentricity
**     aorl    double     mean anomaly or longitude (radians, jform=1,2 only)
**     dm      double     daily motion (radians, jform=1 only)
**
**  Returned:
**     pv      double[6]  heliocentric x,y,z,xdot,ydot,zdot of date,
**                                       J2000 equatorial triad (AU,AU/s)
**     jstat   int        status:  0 = OK
**                                -1 = illegal jform
**                                -2 = illegal e
**                                -3 = illegal aorq
**                                -4 = illegal dm
**                                -5 = failed to converge
**
**  Notes
**
**  1  date is the instant for which the prediction is required.  It is
**     in the TT timescale (formerly Ephemeris Time, ET) and is a
**     Modified Julian Date (JD-2400000.5).
**
**  2  The elements are with respect to the J2000 ecliptic and equinox.
**
**  3  Three different element-format options are available:
**
**     Option jform=1, suitable for the major planets:
**
**     epoch  = epoch of elements (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = longitude of perihelion, curly pi (radians)
**     aorq   = mean distance, a (AU)
**     e      = eccentricity, e
**     aorl   = mean longitude L (radians)
**     dm     = daily motion (radians)
**
**     Option jform=2, suitable for minor planets:
**
**     epoch  = epoch of elements (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = argument of perihelion, little omega (radians)
**     aorq   = mean distance, a (AU)
**     e      = eccentricity, e
**     aorl   = mean anomaly M (radians)
**
**     Option jform=3, suitable for comets:
**
**     epoch  = epoch of perihelion (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = argument of perihelion, little omega (radians)
**     aorq   = perihelion distance, q (AU)
**     e      = eccentricity, e
**
**  4  Unused elements (dm for jform=2, aorl and dm for jform=3) are
**     not accessed.
**
**  5  The reference frame for the result is equatorial and is with
**     respect to the mean equinox and ecliptic of epoch J2000.
**
**  6  The algorithm is adapted from the EPHSLA program of
**     D.H.P.Jones (private communication, 1996).  The method is
**     based on Stumpff's Universal Variables;  see Everhart and
**     Pitkin (1983, Am.J.Phys.51,712).
**
**  Last revision:   26 June 1997
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

/* Gaussian gravitational constant (exact) */
#define GCON 0.01720209895

/* Canonical days to seconds */
#define CD2S ( GCON / 86400.0 )

/* Sin and cos of J2000 mean obliquity (IAU 1976) */
#define SE 0.3977771559319137
#define CE 0.9174820620691818

/* Test value for solution and maximum number of iterations */
#define TEST 1e-11
#define NITMAX 20

{
   int nit, n;
   double am, w, pht, argph, q, tf, alpha, aba, sqa,
          dt, fc, fp, psi, psj, beta, bs0, bs1, bs2, bs3,
          ff, fdot, phs, sw, cw, si, ci, so, co,
          x, y, z, xdot, ydot, zdot, vf;


/* Validate arguments. */
   if ( jform < 1 || jform > 3 ) {
      *jstat = -1;
      return;
   }
   if ( e < 0.0 || ( e >= 1.0 && jform != 3 ) ) {
      *jstat = -2;
      return;
   }
   if ( aorq <= 0.0 ) {
      *jstat = -3;
      return;
   }
   if ( jform == 1 && dm <= 0.0 ) {
      *jstat = -4;
      return;
   }

/*
** Transform elements into standard form:
**
** AM    = mean anomaly (M)
** PHT   = epoch of perihelion passage
** ARGPH = argument of perihelion (little omega)
** Q     = perihelion distance (q)
**
** Also computed is TF, the ratio of the daily motion to
** the "theoretical" value.
*/

   switch ( jform ) {

   case 1:
      am = aorl - perih;
      w = sqrt ( aorq * aorq * aorq ) / GCON;
      pht = epoch - am * w;
      argph = perih - anode;
      q = aorq * ( 1.0 - e );
      tf = dm * w;
      break;

   case 2:
      am = aorl;
      pht = epoch - am * sqrt ( aorq * aorq * aorq ) / GCON;
      argph = perih;
      q = aorq * ( 1.0 - e );
      tf = 1.0;
      break;

   case 3:
      am = aorl;
      pht = epoch;
      argph = perih;
      q = aorq;
      tf = 1.0;

   }

/* The universal variable alpha.  This is proportional to the total */
/* energy of the orbit:  -ve for an ellipse, zero for a parabola,   */
/* +ve for a hyperbola.                                             */

   alpha = ( e - 1.0 ) / q;
   aba = fabs ( alpha );
   sqa = sqrt ( aba );

/* Time from perihelion to date (in Canonical Days:  a canonical day */
/* is 58.1324409... days, defined as 1/GCON).  The portion of the    */
/* time from the epoch to date is adjusted to take account of any    */
/* difference between the supplied and theoretical daily motion.     */

   dt = ( ( date - epoch ) * tf + epoch - pht ) * GCON;

/* First Approximation to the Universal Eccentric Anomaly, psi, */
/* based on the circle (fc) and parabola (fp) values.           */

   fc = dt / q;
   w = pow ( 3.0 * dt + sqrt ( 9.0 * dt * dt + 8.0 * q * q * q ),
             1.0 / 3.0 );
   fp = w - 2.0 * q / w;
   psi = ( 1.0 - e ) * fc + e * fp;

/* Successive Approximations to Universal Eccentric Anomaly. */

   nit = 1;
   w = 1.0;
   while ( fabs ( w ) >= TEST ) {

   /* Compute the Universal Variables bs0, bs1, bs2, bs3. */

   /* Form half angles until beta below maximum (0.7). */
      n = 0;
      psj = psi;
      beta = alpha * psi * psi;
      while ( fabs ( beta ) > 0.7 ) {
         n++;
         beta /= 4.0;
         psj /= 2.0;
      }

   /* Calculate Universal Variables by nested series. */
      bs3 = psj * psj * psj * ( ( ( ( ( ( beta / 210.0 + 1.0 )
                                        * beta / 156.0 + 1.0 )
                                        * beta / 110.0 + 1.0 )
                                        * beta / 72.0 + 1.0 )
                                        * beta / 42.0 + 1.0 )
                                        * beta / 20.0 + 1.0 ) / 6.0;
      bs2 = psj * psj * ( ( ( ( ( ( beta / 182.0 + 1.0 )
                                  * beta / 132.0 + 1.0 )
                                  * beta / 90.0 + 1.0 )
                                  * beta / 56.0 + 1.0 )
                                  * beta / 30.0 + 1.0 )
                                  * beta / 12.0 + 1.0 ) / 2.0;
      bs1 = psj + alpha * bs3;
      bs0 = 1.0 + alpha * bs2;

   /* Double angles until n vanishes. */
      while ( n > 0 ) {
         bs3 = 2.0 * ( bs0 * bs3 + psj * bs2 );
         bs2 = 2.0 * bs1 * bs1;
         bs1 = 2.0 * bs0 * bs1;
         bs0 = 2.0 * bs0 * bs0 - 1.0;
         n--;
         psj *= 2.0;
      }

   /* Improve the approximation. */
      ff = q * bs1 + bs3 - dt;
      fdot = q * bs0 + bs2;
      w = ff / fdot;
      psi -= w;
      if ( nit < NITMAX ) {
         nit++;
      } else {
         *jstat = -5;
         return;
      }
   }

/* Speed at perihelion. */

   phs = sqrt ( alpha + 2.0 / q );

/*
** In a Cartesian coordinate system which has the x-axis pointing
** to perihelion and the z-axis normal to the orbit (such that the
** object orbits counter-clockwise as seen from +ve z), the
** perihelion position and velocity vectors are:
**
**   position   [Q,0,0]
**   velocity   [0,PHS,0]
**
** Using the Universal Variables we project these vectors to the
** given date:
*/

   x = q - bs2;
   y = phs * q * bs1;
   xdot =  - bs1 / fdot;
   ydot = phs * ( 1.0 - bs2 / fdot );

/*
** To express the results in J2000 equatorial coordinates we make a
** series of four rotations of the Cartesian axes:
**
**           axis      Euler angle
**
**     1      z        argument of perihelion (little omega)
**     2      x        inclination (i)
**     3      z        longitude of the ascending node (big omega)
**     4      x        J2000 obliquity (epsilon)
**
** In each case the rotation is clockwise as seen from the +ve end
** of the axis concerned.
*/

/* Functions of the Euler angles. */
   sw = sin ( argph );
   cw = cos ( argph );
   si = sin ( orbinc );
   ci = cos ( orbinc );
   so = sin ( anode );
   co = cos ( anode );

/* Position. */
   w = x * cw - y * sw;
   y = x * sw + y * cw;
   x = w;
   z = y * si;
   y = y * ci;
   w = x * co - y * so;
   y = x * so + y * co;
   pv [ 0 ] = w;
   pv [ 1 ] = y * CE - z * SE;
   pv [ 2 ] = y * SE + z * CE;

/* Velocity (scaled to AU/s and adjusted for daily motion). */
   w = xdot * cw - ydot * sw;
   ydot = xdot * sw + ydot * cw;
   xdot = w;
   zdot = ydot * si;
   ydot = ydot * ci;
   w = xdot * co - ydot * so;
   ydot = xdot * so + ydot * co;
   vf = tf * CD2S;
   pv [ 3 ] = vf * w;
   pv [ 4 ] = vf * ( ydot * CE - zdot * SE );
   pv [ 5 ] = vf * ( ydot * SE + zdot * CE );

/* Finished. */
   *jstat = 0;
   return;
}
