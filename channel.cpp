
/************
*Definitions of functions for class Channel*
*Definition of functions for classes Junction1, Junction2, and Junction3
*includes adjoint variable storage/solvers
*************************************/
#include "channel.h"

/*
////
double Ptheta(double A){
	//This polynomial interpolates [0, 0.95*Af]:
	//double theta = -51373.9446484794*pow(A,10)+214568.573482387*pow(A,9)
	//+-386661.433789624*pow(A,8)+393840.3676702*pow(A,7)+-249498.681803086*pow(A,6)+
	//101951.704467777*pow(A,5)+-26998.0616861897*pow(A,4)+4553.1074838931*pow(A,3)+
	//-474.700106183024*pow(A,2)+34.1596999465758*A+0.446951718804595;
	//This polynomial interpolates [0.1*A/D, 0.95*Af]
	double theta =1927.8569448950*A*A*A *A*A*A *A*A*A +-6160.4825286065*A*A*A *A*A*A *A*A +8423.8271813105*A*A*A *A*A*A*A 
	+-6430.9755770757*A*A*A*A*A*A +3009.6548102583*A*A*A*A*A +-893.1759921281*A*A*A*A 
	+169.9339821843*A*A*A +-20.2421601712*A*A +2.8514566220*A +3.6440131732;
	return theta*pow(A,1./3.);
}
*/
/** retrun theta as a function of A, where A = D^2/8(theta-sin(theta))*/

inline double getTheta(double A, double D)
{
	int count;
	ftheta th(A,D);
	double theta =::ridders(th, -.1, 2*PI+1, &count,1e-15, 1e-15);
//	printf("errr in theta  is %e\n", fabs(A-D*D/8*(theta-sin(theta))));	

//this way is 3x as fast--but! lose a lot of accuracy near the top.
//	double theta = Ptheta(A/(D*D));
//	for(int i=0;i<2; i++)
//	{theta = (A>0.)? (theta -(theta-sin(theta)- 8.*A/(D*D))/(1.-cos(theta))):0.;}
	return theta;
}

/*unstable and possibly  terrible way to comput Phi in the circular part of the pipe*/

//inline double powPhi(double t, double D){
//	return sqrt(G*3.*D/8.)*(t-1/80.*t*t*t+19./448000.*t*t*t*t*t
//		+1./10035200.*t*t*t*t*t*t*t+491./(27.*70647808000.)*t*t*t*t*t*t*t*t*t);}


/**Phi = \int_0^A (c(z)/z)dz, where c(z) is the gravity wavespeed as a function of A
 * A = area, D = diameter, At = transition area, Ts = slot width, P = true if pressurized, false otherwise
 */

const int Ncheb = 20;
const double xx []= {-1.0000000000000000,  -0.9876883405951378,  -0.9510565162951535,  -0.8910065241883679,  
		     -0.8090169943749475,  -0.7071067811865476,  -0.5877852522924732,  -0.4539904997395468,  
		     -0.3090169943749475,  -0.1564344650402309,  -0.0000000000000000,  0.1564344650402306,  
		      0.3090169943749473,  0.4539904997395467,  0.5877852522924730,  0.7071067811865475,  
		      0.8090169943749473,  0.8910065241883678,  0.9510565162951535,  0.9876883405951377, 1.0000000000000000, };

//polynomial coefficients
////for h(A)
const double coeffs_h[]={2.412767243157768248135319100059554e-01, 2.490551009703833796234839617091749e-01, 8.588320472482467973567819078662384e-03,
 			 9.228793157998457405435172914627134e-04, 1.310660974448734367701266018026398e-04, 2.129558025374983921564763711749651e-05,
			 3.749094616467203989458105646535410e-06, 6.962795341462423230912527993610194e-07, 1.343547052451044239960227996180368e-07,
		  	 2.668125626637029475349687868096361e-08, 5.418626817526250261028102778989540e-09, 1.120393624965683396198531943326663e-09,
		  	 2.350950901068854991515379247450580e-10, 4.994029090156534158917749943719349e-11, 1.071960458760750008323825025497721e-11,   
		     	 2.321591726189518874185799075341815e-12, 5.067116805835826216315004584447364e-13, 1.113605910211862088384776819368985e-13,
		  	 2.467624478577450438100243851682381e-14, 5.743999679585741830689301264332563e-15, 1.221321604072796243118941605439226e-15};

//for phi(A)  when 0<A<=pi/8  
const double coeffs_p1[] = {2.6099703290809577, 2.6418910954954722, 0.0407198075367772, 0.0101962412156085, 0.0017585298273079,
       			0.0004510739954107, 0.0001148009501627, 0.0000322213950326, 0.0000093014159914, 0.0000027886790457, 
			0.0000008553527957, 0.0000002678904977, 0.0000000852610460, 0.0000000275113722, 0.0000000089810480, 
			0.0000000029602467, 0.0000000009847978, 0.0000000003306886, 0.0000000001120956, 0.0000000000423243, 0.0000000000127085 };
//for phi(A) when pi/8<=A<=pi
const double coeffs_p2[]= {6.4670122383491355, -0.8635238603246120, -0.2591109614939733, -0.0292512185543841, -0.0076221992053693,
       			-0.0017389019022193, -0.0004503493365253, -0.0001208472298793, -0.0000332374280799, -0.0000093988438938, 
			-0.0000027068870730, -0.0000007850335833, -0.0000002345070328, -0.0000000682526355, -0.0000000214757085, 
			-0.0000000059227336, -0.0000000021646739, -0.0000000004279834, -0.0000000002981759, 0.0000000000305111, -0.0000000000597102};
//for A(phi) when phi<phi(pi/8)
const double coeffs_a1[] = {0.1262575316112871, 0.1873031261944491, 0.0712845714324142, 0.0092391530562048, -0.0011662365223237,
       			-0.0001871946267119, -0.0000251074200750, -0.0000052509280877, -0.0000011451817876, -0.0000002739189119,
		       	-0.0000000680431439, -0.0000000175632951, -0.0000000046503898, -0.0000000012589959, -0.0000000003467156, 
			-0.0000000000969788, -0.0000000000276160, -0.0000000000079714, -0.0000000000024471, -0.0000000000003386, 0.0000000000001585};
//for A(phi) when phi(pi/8)<phi
const double coeffs_a2[] = {0.6277322274489641, -0.2023497520119139, -0.0391435613898155, 0.0060556625096610, 0.0004614027183724,
       			-0.0000553007320124, -0.0000015358044760, -0.0000001139988435, 0.0000000727565380, -0.0000000282843815,
		       	0.0000000124134323, -0.0000000058741562, 0.0000000029710635, -0.0000000015906890, 0.0000000008960590, 
			-0.0000000005293791, 0.0000000003289447, -0.0000000002183606, 0.0000000001557247, -0.0000000001192869, 0.0000000000532795};

//powers for scaling x points before evaluation (determined via python-automated trial and error)
const double ph = 2./3.;
const double pA1 = 1./3.;
const double pA2 = 5./12.;
const double pPhi1 = 1.;
const double pPhi2 = 3./5.;


/*double PhiofAold(double A, double D, double At, double Ts, bool P)
{
	
	double phi = 0.;
	double t=0.;
	if(A<At&& (!P))
	{
		t = getTheta(A,D);
		phi = powPhi(t,D);
	}
	else
	{
		t = getTheta(At, D);
		phi = powPhi(t,D)+2*sqrt(A/Ts)-2*sqrt(At/Ts);
	}
	return phi;
}*/

double HofA(double A, double D, double At, double Ts, bool P)
{
	double y; 
	if(A<1e-15){return 0.;}
	if(A<=At && !P)  //below slot
	{
		A = A/(D*D);//normalize by full area;
		if (A<=PI/8.){
			double Ahat = 2*pow((A*8./PI),ph)-1.;
			y = D*ChebEval(&coeffs_h[0],Ncheb, Ahat);
		}
		else{
			double Ahat = 2*pow(8./PI*(PI/4.-A),ph)-1.;
			y = D*(1.- ChebEval(&coeffs_h[0],Ncheb, Ahat));

		}
	}
	else      //in Preissman Slot
	{
		y = D+(A-At)/Ts;
		//cout<<"y = "<<y<<" A = "<<A<<endl;
	}
	return y;
}

double PhiofA(double A, double D, double At, double Ts, bool P)
{
	if(A<1e-15){return 0.;}
	double phi = 0.;
	if(A<=At)
	{	
		A = A/(D*D);
		if(A<PI*D*D/8.)
		{	
			double Ahat = 2*pow((A*8./PI),pA1)-1.;
			phi = D*ChebEval(&coeffs_p1[0],Ncheb, Ahat);
		}
		else
		{
			double Ahat = 2.*pow((PI/4.-A)*8/PI,pA2)-1;
			phi = D*ChebEval(&coeffs_p2[0], Ncheb, Ahat);
		}
	}
	else
	{
		phi = D*ChebEval(&coeffs_p2[0], Ncheb, -1)+2.*(sqrt(A/Ts)-sqrt(At/Ts));

	}
	return phi;
}

double AofPhi(double phi, double D, double At, double Ts, bool P)
{
	if(phi<1e-15){return 0.;}
	double A = 0.;
	double phi1m = D*ChebEval(&coeffs_p2[0],Ncheb, 1.);
	double phi2m = D*ChebEval(&coeffs_p2[0],Ncheb, -1.);
	if(phi<phi2m&&!P)
	{
		
		double p1 = 1.;
		double p2 = 3./5.;
		if(phi<phi1m)
		{	
			double phat = 2*pow(phi/phi1m,pPhi1)-1.;
			A = D*D*ChebEval(&coeffs_a1[0],Ncheb, phat);
		}
		else
		{
			double phat = 2.*pow((phi2m-phi)/(phi2m-phi1m),pPhi2)-1.;
			A = D*D*ChebEval(&coeffs_a2[0], Ncheb, phat);
		}
	}
	else
	{
		A = D*D*ChebEval(&coeffs_a2[0], Ncheb, -1)+2.*(sqrt(A/Ts)-sqrt(At/Ts));

	}
	return A;

}

/** Cgrav = sqrt(g*A/l)  is the gravity wavespeed (l is the width of the free surface)
 *this routine has some problems near the transition
 * */

double Cgrav(double A, double D, double At, double Ts, bool P)
{
	double c;
	if(A<1e-15){return 0.;}
	if(A<At)
	{
		double h = HofA(A,D,At,Ts,P);
		double l = 2.*sqrt(h/D*(1.-h/D));
		c = sqrt(G*A/l);
	}
	//in slot assign pressure wavespeed sqrt(G*A/Ts) (should be basically a)
	else   
	{
		c = sqrt(G*A/Ts);
	}

	return c;    
}


/**Eta = g*I(A) = g*int_0^h(A) (h-z)l(z) dz */

	/*double y = hofA(A);	
		if (A<=At&& (!Pnow))
		{
			Eta = G/12.*((3.*D*D-4.*D*y+4.*y*y)*sqrt(y*(D-y))
				-3.*D*D*(D-2.*y)*atan(sqrt(y)/sqrt(D-y)));
		//	if(Pnow){Eta-=((A-At)*(A-At)/Ts)/2.;}//bahahahahahahahah this is bullshit...? (YES)
					}
		else 
		{
			Eta = (y-yt)*At+G/12.*(sqrt(yt*(D-yt))*(3.*D*D+2.*D*yt-6.*D*y-8.*yt*yt+12.*yt*y)
				-3.*D*D*(D-2.*y)*atan(sqrt(yt/(D-yt))))+G*Ts*(y-yt)*(y-yt)/2.;
		}*/

double Eta(double A, double D, double At, double Ts, bool P)
{
	double Eta,t;
	if (A<At)
	{
	//	t = getTheta(A,D);
		double y = HofA(A, D, At, Ts, P);
	//	double y = D/2.*(1.+cos(PI-t/2.));
		Eta = G/12.*((3.*D*D-4.*D*y+4.*y*y)*sqrt(y*(D-y))
			-3.*D*D*(D-2.*y)*atan(sqrt(y)/sqrt(D-y)));
	//	double st  = sin(t/2.);
	//	Eta = 1./24.*(3*st-st*st*st-3*t/2*cos(t/2))*G*D*D*D;
	//      printf("Eta_s -Eta_mine = %e\n", fabs(Eta2-Eta));	
	}
	//in slot use Sanders 2011 TPA approach
	else 
	{
		/*old possibly terrible way
		t = gettheta(At,D);
		double yt = D/2.*(1.+cos(PI-t/2.));
		y = yt+(A-At)/Ts;
		Eta = (y-yt)*At+G/12.*(sqrt(yt*(D-yt))*(3.*D*D+2.*D*yt-6.*D*y-8.*yt*yt+12.*yt*y)
			-3.*D*D*(D-2.*y)*atan(sqrt(yt/(D-yt))))+G*Ts*(y-yt)*(y-yt)/2.;*/
		double H = (A-(PI*D*D/4.))/Ts;
		H = (A-At)/Ts;
		Eta = PI/4.*G*D*D*(H+D/2.);
	}
	return Eta;
}



//Channel definitions
////
//
//Constructor
Channel::Channel(int Nin, double win, double Lin, int Min, double a):N(Nin), w(win), L(Lin),kn(1.0), M(Min) 
{
	q0 = new double[2*N];
	q = new double[2*N];
	qhat = new double[2*N];
	bfluxleft = new double[2];
	bfluxright = new double[2];
	
	//assume junctions aren't ventilated unless they're junction1s without reflection
	P.push_back(false);
	for(int i = 0; i<N+1; i++){P.push_back(false);}
	P.push_back(false);
	n = 0;
	if(N*M<1e7){
		q_hist = new double[2*(N+2)*(M+2)];
	}
	else 
	{
		cout<<"You know, allocating "<< sizeof(w)*M*N*2<<" bytes may be a bad idea! Decrease time increment or figure out a better storage scheme.\n";
		throw("Yeah...nope.");
	}

//try it with ghost cells...I'm desparate...
	Mr = 0.;
	S0 = 0.;	
	int i,j;
	dx = L/((double)N);
	for(i=0; i<2*N; i++)
	{
		q0[i]=0.;
		q[i] =0.;
		qhat[i] = 0.;
	}
	for(j= 0; j<2;j++)
	{
		bfluxleft[j] = 0;
		bfluxright[j] = 0;
	}

}

//Destructor
Channel::~Channel()
{
	delete [] q0;
	delete [] q;
	delete [] q_hist;
	delete [] qhat;
	delete [] bfluxleft;
	delete [] bfluxright;
}

//Display geometry info about this pipe
void Cuniform::showGeom()
{
	printf("\nAbout this channel:\n");
	cout<<"Cross section is uniform\n";
	printf("|         |\n|<---w--->|\n|_________|\n");	
	printf("w = %2.1f %21s width (m)\nN = %4d %20s number of grid points\nL = %3.1f %20s length (m)\n", w,"",N,"",L,"");
	printf("Mr = %1.3f %20s Manning roughness coefficient\nS0 = %1.2f %22s Bed slope\n", Mr, "", S0, "");
}

void Cpreiss::showGeom()
{
	printf("About this channel:\n\n");
	cout<<"Cross section is Preissman Slot\n";
	printf("     Ts\n     <->\n     | |\n     | |      \n  \/~~~~~~~\\    ^\n \/         \\   |\n(<----D---->)  |  yt\n \\         \/   |\n  \\_______\/    v\n");	
	printf("D = %.1f %20s width (m) \nN = %d %20s number of grid points \nL = %.1f %20s length (m)\n", w,"",N,"",L,"");
	printf("Slot width Ts= %1.5f\nTransition height yt= %1.5f\n",Ts,yt);
	printf("Mr = %1.3f %20s Manning Roughness coeff\nS0 = %1.2f %20s bed slope\n", Mr, "", S0, "");
}


/**Display current cell values - argument is 0 for q0, 1 for q*/
void Channel::showVals(int Iwantq)
{

	int i;
	if(Iwantq)
	{
		for(i=0; i<N; i++)
		{
			printf("A0[%d] = %f and Q0[%d] = %f\n", i,q[idx(0,i)],i, q[idx(1,i)]); 
		}
	}
	else
	{
		for(i=0; i<N; i++)
		{
			printf("A[%d] = %f and Q[%d] = %f\n", i,q0[idx(0,i)],i, q0[idx(1,i)]); 
		}
	}	
}
/**show pressure*/
void Cuniform::showp()
{

	int i;
	for(i=0; i<N; i++)
	{
		printf("h[%d] = %f and Q0[%d] = %f\n", i,q[idx(0,i)]/w,i, q[idx(1,i)]); 
	}
}
/**show pressure values*/
void Cpreiss::showp()
{

	int i;
	double h;
	for(i=0; i<N; i++)
	{
		h = fakehofA(q[idx(0,i)], P[pj(i)]);
		printf("pressure head[%d] = %f (m) = %f (psi) and Q0[%d] = %f, P = %s\n", i,h,m_to_psi*h,i, q[idx(1,i)],P[pj(i)]?"true":"false"); 
	}
}


/**Initialize q0 with constant data (A0,Q0)*/
void Channel::setq0(double A0, double Q0) 
{
	int i;
	for(i=0; i<N; i++)
	{
		q0[idx(0,i)] = A0;
		q0[idx(1,i)] = Q0;
		q_hist[idx_t(0,i+1,0)] = A0;
		q_hist[idx_t(1,i+1,0)] = Q0;
		P[pj(i)] = A0>=At;
	}
	q_hist[idx_t(0,0,0)] = A0;
	q_hist[idx_t(0,N+1,0)] = A0;
	q_hist[idx_t(1,0,0)] = Q0;
	q_hist[idx_t(1,N+1,0)] = Q0;
}
/**initialize with nonconstant data A0, Q0*/
void Channel::setq0(double *A0, double *Q0)
{
	int i;
	for(i=0; i<N; i++)
	{
		q0[idx(0,i)] = A0[i];
		q0[idx(1,i)] = Q0[i];
		q[idx(0,i)] = A0[i];
		q[idx(1,i)] = Q0[i];
		q_hist[idx_t(0,i+1,0)] = A0[i];
		q_hist[idx_t(1,i+1,0)] = Q0[i];
		P[pj(i)] = A0[i]>=At;
	}
	if(A0[0]>=At){
	P[pj(0)] = true;
	}
	if (A0[N-1]>=At){	
	P[pj(N)] = true;	
	}
	q_hist[idx_t(0,0,0)] = A0[0];
	q_hist[idx_t(0,N+1,0)] = A0[N-1];
	q_hist[idx_t(1,0,0)] = Q0[0];
	q_hist[idx_t(1,N+1,0)] = Q0[N-1];


}

void Channel::setq(vector<double>A0, vector<double>Q0)
{
	int i;
	for(i=0; i<N; i++)
	{
		q0[idx(0,i)] = A0[i];
		q0[idx(1,i)] = Q0[i];
		q[idx(0,i)] = A0[i];
		q[idx(1,i)] = Q0[i];
		q_hist[idx_t(0,i+1,0)] = A0[i];
		q_hist[idx_t(1,i+1,0)] = Q0[i];
		P[pj(i)] = A0[i]>=At;
	}
	q_hist[idx_t(0,0,0)] = A0[0];
	q_hist[idx_t(0,N+1,0)] = A0[N-1];
	q_hist[idx_t(1,0,0)] = Q0[0];
	q_hist[idx_t(1,N+1,0)] = Q0[N-1];
}

/**Initialize q with constant data (A0,Q0)*/
void Channel::setq(double A0, double Q0)
{
	int i;
	for(i=0; i<N; i++)
	{
		q[idx(0,i)] = A0;
		q[idx(1,i)] = Q0;
		P[pj(i)] = A0>=At;
	}
}		

/////**take M Euler steps of length dt to update conservation law for left state [q1(i), q2(i)] and right state [q1p(i+1), q2(i+1)],using numflux as numerical flux*/
void Channel::stepEuler(double dt)
{
	cmax = 0;
	double fplus[2] ={0};
	double fminus[2] ={0};
	double nu = dt/dx;				
	double negtol = -dx/10.;		//decide how negative A forces the code throws an error over negative area (vs. just printing a warning) 
	int i,k;
	//update leftmost cell using externally assigned fluxes bfluxleft
	//get pressurization information for this cell
	Pnow = P[pj(0)]; 							              
	numFlux(q0[idx(0,0)], q0[idx(0,1)], q0[idx(1,0)], q0[idx(1,1)],fplus,P[pj(0)], P[pj(1)]);
	q[idx(0,0)] = q0[idx(0,0)]-nu*(fplus[0]-bfluxleft[0]);           
	q[idx(1,0)] = q0[idx(1,0)]-nu*(fplus[1]-bfluxleft[1]);	
	P[pj(0)] = (q[idx(0,0)]>At )||( P[pj(-1)]==true && P[pj(1)]==true); 
	for(i = 1; i<N-1; i++)
	{
 		// - fluxes are the previous step's + fluxes
		fminus[0] = fplus[0];
		fminus[1] = fplus[1];                                                       
		Pnow = P[pj(i)];		
		// + fluxes need to be computed afresh
		numFlux(q0[idx(0,i)], q0[idx(0,i+1)], q0[idx(1,i)], q0[idx(1,i+1)],fplus, P[pj(i)], P[pj(i+1)]);                    
		// update conservation law
		for(k=0; k<2; k++)
		{
			q[idx(k,i)] = q0[idx(k,i)]-nu*(fplus[k]-fminus[k]);
		}                    
		//update pressurization info: if A>At-> pressurized pipe; if A<At AND a neighbor is free surface, pipe is free surface
		P[pj(i)] = (q[idx(0,i)]>At )||( P[pj(i-1)]==true && P[pj(i+1)]==true);
	}
	// update rightmost cell using externally assigned fluxes bfluxleft
	P[pj(N-1)] = (q[idx(0,N-1)]>At )||( P[pj(N-2)]==true && P[pj(N)]==true); 
	q[idx(0, N-1)] = q0[idx(0,N-1)] - nu*(bfluxright[0]-fplus[0]);                      
	q[idx(1,N-1)] = q0[idx(1,N-1)] - nu*(bfluxright[1] - fplus[1]);	
	//fold in source terms
	stepSourceTerms(dt);
	// set q0 to updated value
	for(i =0;i<N;i ++)
	{
		for(k = 0; k<2; k++)
		{
			q0[idx(k,i)] = q[idx(k,i)];
		}                                           
		//check for negative areas --do I really need this?
		if(q0[idx(0,i)]<0)
		{
			printf("!!Negative area!!!\n with a[%d] = %f at time %f\n ", i, q0[i], dt*(double)n);
		}
		if (q0[idx(0,i)]<negtol)
		{
			q0[idx(0,i)] = 0.0;
			throw "Oh damn. Negative area!";
		}
	}
	//printf("cmax =%f and CFL=%f",cmax, dt/dx*cmax);
	cmax = 0.;  //reset CFL;

}

/** specific physical flux for Preissman slot*/
void physFluxBetter(double A, double Q, bool P, double D, double At, double Ts, double *flux)
{
	flux[0] = Q;
	flux[1] = (A>0? Q*Q/A:0.) +Eta(A, D, At, Ts, P); 
}
 // Exact Riemann flux, my best guess of implementation in Kerger 2011. Seems unstable for supercritical flow, not sure why.
void Channel::numFluxExact(double q1m, double q1p, double q2m, double q2p, double *flux, bool Pl, bool Pr)
{
	double qnew[2];
	bool Px=Pnow; //if and right are both pressurized, then so is Px; else it's not pressurized
	updateExactRS(q1m, q1p, q2m, q2p, qnew, Pl, Pr, Px);//sample solution at dx/dt = 0 (along t-axis)
	physFluxBetter(qnew[0], qnew[1], Px, w, At, Ts, flux);
}

/**update the solution by samlping exact solution to Riemann problem with q(left) = [q1m, q2m] and q(right) = [q1p, q2p] */
void Cpreiss::updateExactRS(double q1m, double q1p, double q2m, double q2p, double *qnew, bool Pl, bool Pr, bool Px){
	/* * what's going on here:
	 *     ?    :     ?                   ?  ?    :
	 *      \   u*   /                    ` u*\   :
	 *       \  :   /                       `  \  :
	 *    um  \ :  / up                  um   ` \ :up
	 * ________\:/_______  or maybe    _________`\:_________ or even...
	 *          :                                 : 
	 * ? denotes either shock or rarefaction
	 * */
	//Astar and Qstar are the values of (A,Q) in the "star" region in between qm and qp
	double up, um, Astar, Amax, Qstar, ustar;
	um = (q1m>0.? q2m/q1m : 0.);
    	up = (q1p>0.? q2p/q1p : 0.);
	int count;
	//compute u* = [Astar, Qstar]
	Amax = fmax(fmax(q1p, q1m), 1.001*At/.95);     //max of nearby values and 2*Af (for root bracketing)
	flr fl(w, Ts, At, q1m, Pl, Px);
	flr fr(w, Ts, At, q1p, Pr, Px);
	f_exactRS ff(fl, fr, um, up);
	printf("about to solve for Astar with qm = [%f, %f] and qp = [%f,%f] ul = %f, ur = %f\n", q1m, q2m, q1p, q2p, um, up);
	Astar = ::ridders(ff, 0, Amax, &count,1e-10, 1e-12);
	double cbar = (Cgrav(q1m,Pl) +Cgrav(q1p,Pr))/2.;
	double Astar1 = (q1m+q1p)/2.*(1+(um-up)/(PhiofA(q1p,Pl)+PhiofA(q1m,Pr)));
	double Astar2 = (q1m+q1p)/2.*(1+( (cbar>1e-6)? (um-up)/(2.*cbar): 0));
	Qstar = Astar/2.*(up+um)+Astar/2.*(-fl(Astar)+fr(Astar));
	cout<<"Astar = "<<Astar<<"  f(A*) = "<<ff(Astar)<<" Aapprox = "<<Astar1<<" ff(Aapprox)="<<ff(Astar1)<<"Alin = "<<Astar2<<" f(Alin) = "<<(Astar2>0?ff(Astar2):42)<<endl;
	cout<<"fl(Astar)="<<fl(Astar)<<" fr(Astar)="<<fr(Astar)<<endl;
	ustar = Astar>0? Qstar/Astar:0;
	printf("ql = [%f, %f], qr = [%f, %f], q* = [%f, %f]\n", q1m, q2m, q1p, q2p, Astar, Qstar);
	double sl, sr;
	//now evaluate speeds and sample solution at dx/dt = 0 (along t-axis) 
	//left side 
	if(Astar<q1m)//rarefaction
	{
		
		printf("left rarefaction-- ");
		double sl1, sl2; //bounding edges of rarefaction fan
		sl1 = um - Cgrav(q1m, Pl);
		sl2 = ustar - Cgrav(Astar,Px);
		double m1 = fmin(sl1,sl2);
		double m2 = fmax(sl1, sl2);
		if (m1<0 && m2>0)//omg we're in the rarefaction!!!
		{
			//assume u-c = x/t = 0 (evaluate on x-axis). Then assume u+phi(A) = ul+phi(Al) (Riemann invariant is const)
			//so nonlinear solve here: c(x)+phi(x)-ul-phi(Al) = 0
			double Ah;
			double lhs = um +PhiofA(q1m,Pl);
			fallpurpose fri(w,At,Ts,lhs,0., 1, 0., 1.,Px);
			Ah = ::ridders(fri,0, Amax, &count,1e-10, 1e-12);
			qnew[0] = Ah;
			qnew[1] = Cgrav(Ah,Px)*Ah;
			printf("in rarefaction!");
		}
		else if (m2<=0)//whole rarefaction wave is tilted off to the left
		{
			qnew[0] = Astar;
			qnew[1] = Qstar;
			printf("titled to left\n");
		}
		else //whole thing is tilted off to the right
		{
			qnew[0] = q1m;
			qnew[1] = q2m;
			printf("titled to right\n");
		}
		sl = m1;
		printf("left speeds are sl1 = %f and sl2 = %f\n", sl1, sl2);

	}
	else //shock!!!!
	{
		printf("left shock! speed = ");
			
		sl = um - findOmega(Astar,q1m, Px, Pl);
		
	//	sl = um-sqrt(G*(eEta(Astar, w,At,Ts,Px)-eEta(q1m,w,At,Ts,Px)*Astar)/(Astar-q1m));
		cout<<sl<<endl;
		if(0<=sl)//left shock has positive speed
		{
			qnew[0] = q1m;
			qnew[1] = q2m;
		}
		else
		{
			qnew[0] = Astar;
			qnew[1] = Qstar;
		}
	}

//right side
	if(Astar<q1p)//rarefaction
	{
		printf("right rarefaction-- ");
		double sr1, sr2;
		sr1 = ustar + Cgrav(Astar,Px);
		sr2 = up    + Cgrav(q1p  ,Pr);
		double m1 = fmin(sr1,sr2);
		double m2 = fmax(sr1,sr2);

		if(m1<0 &&m2>0) //omg we're in the rarefaction!!!
		{
		
			//assume u+c = x/t = 0 (evaluate on x-axis). Then assume u-phi(A) = ur-phi(Ar) (Riemann invariant is const)
			//so nonlinear solve here: -c(x)-phi(x)-ur+phi(Ar) = 0
			double Ah;
			double lhs = up -PhiofA(q1p,Pr);
			fallpurpose fri(w,At,Ts,lhs,0., -1, 0., -1.,Px);
			Ah = ::ridders(fri, 0, Amax, &count,1e-10, 1e-12);
			qnew[0] = Ah;
			qnew[1] = -Cgrav(Ah,Px)*Ah;
			printf("in rarefaction!, A= %f, Q = %f", Ah, qnew[1]);
		}
		else if (m2<=0)//whole rarefaction wave is titled off to the left
		{
			qnew[0] = q1p;
			qnew[1] = q2p;
			printf("titled to left\n");
		}
		else//whole thing is tilted off to the right
		{
			qnew[0] = Astar;
			qnew[1] = Qstar;
			printf("titled to right\n");
		}
		sr = m2;
		printf("right speeds are sr1 = %f and sr2 = %f\n", sr1, sr2);
	}
	else//shock!!
	{
		
		printf("right shock! speed = ");
		sr = up+findOmega(Astar, q1p, Px, Pr);
	//	sr = up+sqrt(G*(eEta(Astar, w,At,Ts,Px)-eEta(q1p,w,At,Ts,Px)*Astar)/(Astar-q1p));
		cout<<sr<<endl;
		if(sr>=0) //right shock has positive speed
		{
			qnew[0] = Astar;
			qnew[1] = Qstar;
		}
		else{
			qnew[0] = q1p;
			qnew[1] = q2p;
		}
		cmax = max(cmax, max(fabs(sl),fabs(sr)));
	}
}

 //this is HLL flux. It sucks.
void Channel::numFluxHLL(double q1m, double q1p, double q2m, double q2p, double *flux, bool Pm, bool Pp)                
{
    
	double s[2]={0,0};
	double slow = 1e-5;
	//estimate speeds (Roe or fancier)
//	printf("arguments to speeds are (%f,%f,%f,%f,%p,%d,%d", q1m, q1p, q2m, q2p, s, Pm, Pp);
	speeds(q1m, q1p, q2m, q2p, s, Pm, Pp);
	// check for near zero speeds
	if (fabs(s[0])< slow && fabs(s[1])<slow){                                                       
	flux[0] = 0;
	flux[1] = 0;
	}
	// Update with HLL flux
	else{                                                                                           
        	if(s[0]>=0){
		physFlux(q1m,q2m,flux, Pp);
		}
        else if(s[0]<0 && s[1]>0)
	{
		double Fl[2];
		double Fr[2];
		physFlux(q1m,q2m,Fl, Pm);
		physFlux(q1p,q2p,Fr, Pp);
		flux[0] = (s[1]*Fl[0]-s[0]*Fr[0]+s[0]*s[1]*(q1p-q1m))/(s[1]-s[0]);
		flux[1] = (s[1]*Fl[1]-s[0]*Fr[1]+s[0]*s[1]*(q2p-q2m))/(s[1]-s[0]);
		//cout<<"fluxes are "<<flux[0]<<" "<<flux[1]<<Fl[1]<<" "<<Fr[1]<<endl;
	}
        else if(s[1]<=0)
        	physFlux(q1p, q2p, flux, Pp);
        else
          	printf("Error! Check your speeds. Something is wrong! s = [%f,%f]\n",s[0], s[1]);
     }
    return;
}

/**de St. Venant flux function (See Leon 2009)
 *This is valid for either cuniform (uniform width) or cpreiss (Preissman slot) channel classes
 * */
void Channel::physFlux(double q1, double q2, double *flux, bool P)                  
{
    flux[0] = q2;
    flux[1] = (q1>0? q2*q2/q1:0.) +pbar(q1,P);  
    //cout<<"P is "<<P<<"q1 is"<<q1<<"and pbar is"<<pbar(q1,P)<<endl; 
}

/** HLL speeds--uniform channel*/
void Cuniform::speedsHLL(double q1m, double q1p, double q2m, double q2p, double *s, bool Pm, bool Pp)                           
{
    double um, up, hm, hp, uhat, hhat, smin, smax;
    int j;
    bool p = Pm && Pp;
    um = (q1m>0.? q2m/q1m : 0.);
    up = (q1p>0.? q2p/q1p : 0.);
    hm = HofA(q1m,Pm);
    hp = HofA(q1p,Pp);
    uhat = (hm+hp >0.)? (sqrt(hm)*um+sqrt(hp)*up)/(sqrt(hm)+sqrt(hp)) : 0. ;
    hhat = (hm+hp)/2.;
    Pnow = false;
    smin = Cgrav(hhat*w,p) +uhat;
    smax =  Cgrav(hhat*w,p) +uhat; 
    for(j=0; j<2; j++){
        smin =  min3(um + pow(-1.,j)*Cgrav(hm*w,Pm), uhat + pow(-1.,j)*Cgrav(hhat*w,p),smin);
        smax =  max3(up + pow(-1.,j)*Cgrav(hp*w,Pp),uhat+ pow(-1.,j)*Cgrav(hhat*w,p),smax);
        }
    s[0] = smin;
    s[1] = smax;

   // printf("well shit. s is [%f,%f]\n", s[0],s[1]);
    if(isnan(s[0]) || isnan(s[1]))
    {
	    printf("well shit. s is [%f,%f]\n", s[0],s[1]);
	    printf("qm = %f, qp = %f, um = %f, up = %f,\n", q1m, q1p, up, um); 
    }
}

/**because C89 is too ghetto to have a predefined min function*/
double Channel::min3(double a, double b, double c)                       
{
	return min(min(a,b),c);
}
double Channel::max3(double a, double b, double c)
{
	return max(max(a,b),c);
}

/** Step source term S in u_t+F(u)_x = S as follows:
 * q(t+dt) = q(t) +dt*S(q(t)+dt/2*S(q))
 */
void Channel::stepSourceTerms(double dt){
	int i;
	for (i=0; i<N; i++)
	{
		q[idx(1,i)]= q[idx(1,i)] +dt*getSourceTerms(q[idx(0,i)], q[idx(1,i)] +dt/2.*getSourceTerms(q[idx(0,i)],q[idx(1,i)]));
		q0[idx(1,i)] = q[idx(1,i)];
	}
	
}

/**evaluate source term S - friction slope (Manning Eqn) and actual slope S0 = -dz/dx (S0 should be a non-negative constant)
 *should I have another option for frictions slope?? Eg. Hazen Williams, Darcy-Weishback?
 *in the past this has posed some issues for very small A...be careful...
 * */
double Channel::getSourceTerms(double A, double Q){
	double Sf=0;
	double tol = 1e-1;
	if (A>tol)
	{
		Sf = pow(Mr/kn,2)*Q*fabs(Q)/(A*A*pow(getHydRad(A),4./3.));
	
	}
	return (S0-Sf)*G*A;
}




/**get the volume of water in the channel--just sum up cell values of A multiplied by cell width*/
double Channel::getTheGoddamnVolume()
{
	double evol=0.;
	for(int j = 0; j<N; j++)
	{
		evol  += dx*q[j];
	}
	return evol;
}

//get average spatial gradient at time t_i	
// Simpson's rule... in theory...
double Channel::getAveGradH(int i)
{
	double h1, h2, h3;
	bool p = false;
	h1 = HofA(q_hist[idx_t(0,1,i)],p);
	h2 = HofA(q_hist[idx_t(0,2,i)],p);
	h3 = HofA(q_hist[idx_t(0,3,i)],p);
	double I =0.5*pow((h2-h1)/dx,2);	
	for(int k = 2; k<N-1; k++)
	{
		h1 = h2;
		h2 = h3;
		h3 = HofA(q_hist[idx_t(0,k+1,i)],p);
		I += 2*pow(2.,k%2)*0.5*pow((h3-h1)/(2.*dx),2);
	}
	I += 0.5*pow((h3-h2)/dx,2); 
	I *= dx/3.;
	return I;
}


/**write q information to M/Mi files 
 * these are in theory readable by smartputittogether.py or maybe something else.py...(!???)
 * */
int Channel::writeqToFile(int Mi, double dt)
{	
	
	for(int kk = 0; kk<M+1; kk+=Mi)
	{
		char fname[100];
		snprintf(fname, sizeof fname, "../movie/howdy%03d.txt", kk/Mi);
	//	cout<<fname<<endl;
		FILE *fp = fopen(fname, "w");
		if(fp==NULL)
		{
			fputs("Error opening file!!", stderr);
			return 1;
		}
		//warning: messing with this information will screw up the python plotting script...
		fprintf(fp, " #x   h(A(x))    Q(x)  at t = %f (L = %f, dx = %f, T = %f,  dt = %f ) \n",dt*((float)kk), L,dx, ((double)M)*dt,dt); 
		for(int jj = 0; jj<N+2; jj++)
		{
			bool p = false;
		       fprintf(fp,"%d     %.10f     %.10f\n", jj, HofA(q_hist[idx_t(0,jj,kk)],p),q_hist[idx_t(1,jj,kk)]);
		}
		fclose(fp); 
	}	
	return 0;
}

/**write characteristics Q/A+sign*phi(A) to file in format readable by gnuplot*/
int Channel::writeRItofile(double dt, int sign) 
{
		char fname[100];
		double t,x,Q,A,RI;
		if(sign >0){snprintf(fname, sizeof fname, "../RIpics/RIplus.txt");}
		else{snprintf(fname, sizeof fname, "../RIpics/RIminus.txt");}
		FILE *fp = fopen(fname, "w");
		if(fp==NULL)
		{
			fputs("Error opening file!!", stderr);
			return 1;
		}
		fprintf(fp, "#Riemann invariants u+/-phi, T=%f, L = %f\n", dt*(double)M, L);
		for(int kk = 0; kk<M+1; kk++)
		{
			for(int jj = 0; jj<N+2; jj++)
			{
				t = kk*dt;
				x = (jj-1)*dx;
				A = q_hist[idx_t(0,jj,kk)];
				Q = q_hist[idx_t(1,jj,kk)];
				RI = Q/A +sign*PhiofA(A,false);
				if(sign==0){RI = A;}
		        	fprintf(fp,"%.10f     %.10f     %.10f\n", x, t, RI);
		        	//printf("%.10f     %.10f     %.10f\n", x, t, RI);
			}
			fprintf(fp, "\n");
		}
		fclose(fp); 
	return 0;
}

/**quickly write out some run time info!
 * where is either times or places (if which[k] = 0, then where is a time; else it's a place)
 * K is the length of where
 */
void Channel::quickWrite(double *where, int *which, int K, double T, int skip)
{
	int i, MN;
	double ds;
	for (int k=0; k<K; k++)
	{	
		if(which[k]==0)//write variables at all locations at time where[k] 

		{
			i = round(where[k]*(double)M/K);
			if(i>=M){i=M;}
			if(i<0){i=0;}
			printf("t = %f, index = %d\n", where[k], i);
			MN = N;
			ds = dx;
		}
		else  //write variables at all times at location where[k]
		{
			i = round(where[k]*(double)N/L);
			if(i>=N){i=N-1;}
			if(i<0){i=0;}
			printf("x = %f m, index = %d\n", where[k], i);
			MN = M;
			ds = T/(double)M;
		}
		printf("%s             A               h               Q \n",(which[k])?"t":"x");
		double A, h, hfake, Q;
		for(int j = 0; j<=MN; j+=skip)
		{
			if(which[k]==0)  		
			{
				A = q_hist[idx_t(0,j,i)];
				Q = q_hist[idx_t(1,j,i)];

			}
			else
			{
				A = q_hist[idx_t(0,i,j)];
				Q = q_hist[idx_t(1,i,j)];
				
			}

			h = HofA(A,false);
			hfake = fakehofA(A,true);

			printf(" %f    %f     %f    %f \n", ds*(double)j,A, h, Q);
		}
	}	
}
 /**Set Preissman parameters 
 * a is the pressure wave speed for this pipe -- defaults to 1200
 */
void Cpreiss::setGeom(double a_)

{

//	printf("Ncheb = %d\n", Ncheb);
	a = a_;
	int count;
	Af = PI*D*D/4.;
	Ts = G*Af/(a*a);
//this bit fails epically	
	//ftheta th(At,D);
//	tt =::ridders(th, 0, 2*PI+1, &count,1e-10, 1e-12);

	tt = 2*(PI-asin(Ts/D));

	yt = D/2.*(1-cos(tt/2.));
	At = AofH(yt,false);
//	At = D*D*(tt-sin(tt))/8.;
//	At = .99999*Af;
//	yt = HofA(At,false);	
//	cout<<"D= "<<D<<endl;
//	tt = 2*asin(G*D*2*PI/(8*a*a));// theta such that c(A(theta)) = a
	
	double yt2 = HofA(At, false);
//	cout<<"At = "<<At<<" Ts ="<<Ts<<endl;
//	printf("difference between At and Af is  %e\n", At-PI*D*D/4.);
//	printf("slot gravity wavespeed c  = %f\n", sqrt(D*D*PI/4.*G/Ts));
//	printf("yt = %.16f,At = %.16f, fAt = %.16f AofH(At)-At = %.16f\n",yt, At, AofH(yt,false)-At);
        	
/*	char fname1[30];
	clock_t time1 = clock();
	int duh = (int)time1;
	sprintf(fname1,"geomconfirm%3d.txt",1);
	FILE *fg1 = fopen(fname1,"w");
	int Mp= 500;
	fprintf(fg1, "#D = %f, cgrav = %f, Ts = %.16f, At = %.16f, yt = %.16f \n",D, a, Ts,At,yt); 
	fprintf(fg1, "#A                  h(A)                   I(A)                  c(A)                  phi(A)                  hA(phi(A))                  A(h(A))   htrue(A)\n");
	bool pp = false; 
	for(int k = 0; k<Mp; k++)
	 {
		 //double aa = D*D*PI/(4*Mp)*(double)k;
		 double tt = PI*2/Mp*(double)k;
		 double aa = D*D/8.*(tt-sin(tt));
		 double ht = 0.5*D*(1-cos(tt*.5));
		 double h = HofA(aa,pp);
		 double I = pbar(aa,pp);
		 double ah = AofH(ht,pp);
		 double c = Cgrav(aa,pp);
		 double phi = PhiofA(aa,pp);
		 double ae = AofPhi(PhiofA(aa,pp),pp);
		 fprintf(fg1,"%.16f   %.16f    %.16f   %.16f   %.16f   %.16f   %.16f    %.16f\n", aa, h, I, c, phi, ae, ah, ht); 
	 }
	 fclose(fg1);

	char fname2[30];
       	sprintf(fname2, "geomconfirm%3d.txt", 2);
	FILE *fg2 = fopen(fname2, "w");	
	fprintf(fg2, "A                   h(A)              A-Af\n");
	
	for (int i=0; i<2;i++)
	{
		for(int k = 0; k<40; k++)
		{
		 	double aa =D*D*PI/4.*(1.+pow(-1.,i+1)*pow(2.,-k-1));
		 	double h = HofA(aa,pp);
		 	fprintf(fg2, "%.16f   %.16f   %e   \n", aa, h, aa-D*D*PI/4.);
		} 
	}
	fclose(fg2);*/
}

	
double Cpreiss::AofH(double h, bool p)
{
	if(h<=yt && (!p))
	{	double t = 2.*(acos(1.-h/D*2.));
		return	 D*D*(t-sin(t))/8.;
	}
	else
		return At+Ts*(h-yt);
}
double Cpreiss::pbar(double A, bool p)
{
	double y = HofA(A,p);
	if(y<D && (!p) )
		return  G/12.*((3.*D*D-4.*D*y+4.*y*y)*sqrt(y*(D-y))-3.*D*D*(D-2.*y)*atan(sqrt(y)/sqrt(D-y)));
	else 
		return PI/4.*G*D*D*((A-At)/Ts+D/2.);

}


double Cpreiss::pbar_old(double A, bool P)
	{
		return Eta(A,w, At,Ts, P); 
	}

double Cpreiss::hofAold(double A)
{
		double theta = getTheta(A,D);		
		double y = D/2.*(1.+cos(PI-theta/2.));
		//	cout<<theta<<"= theta and y = "<<y<<endl;
		return y;
}

double Cpreiss::fakehofA(double A, bool p)
{
	if (A>=At){p=true;}
	double y;
	if (p){y = yt+(A-At)/Ts;}	
	else {y = HofA(A,p);}
	return y;
}

double Cpreiss::fakeAofh(double h, bool p)
{
	if (!p){return AofH(h,p);}
	else{return (h-yt)*Ts+At;}	
}

/*
double Cpreiss::Aofh(double h)
	{        
		double A;
		if(h<=yt)  //below transition region
		{
			double theta = 2.*acos(1.-2.*h/D);
			A  = D*D/8.*(theta-sin(theta));
		}
		else
		{ 
			A =At+(h-yt)*Ts;
		}
		return A;
	}

double Cpreiss::thetaofA(double A) //only works for A<polynomial approximation, then two Newton steps to clean
	{
		if(A<=PI*D*D/4.)
		{
		return getTheta(A,D);
		}
		else {
		cout<<"whoops! theta not defined for A>Af"<<endl;
		return 2*PI;}
	}*/

double Cpreiss::getHydRad(double A)
	{	
		double perim;
		if(A<At)
		{
			double y = HofA(A,false);
			double theta = 2*acos(1.-2*y/D);
			perim = D*(2*PI- theta);
		}
		else {perim = D*PI;}
		return A/perim;
	}  


void Cpreiss::speedsRoe(double q1m, double q1p, double q2m, double q2p, double *s, bool Pm, bool Pp)                            // HLL speeds with Roe estimates...
{

    double um, up, hm, hp, uhat, hhat,chat, smin, smax;
    int j;
    bool p = Pm && Pp;
    um = (q1m>0.? q2m/q1m : 0.);
    up = (q1p>0.? q2p/q1p : 0.);
    hm = HofA(q1m,Pm);
    hp = HofA(q1p,Pp);
    uhat = (hm+hp >0.)? (sqrt(hm)*um+sqrt(hp)*up)/(sqrt(hm)+sqrt(hp)) : 0. ;
    hhat = (hm+hp)/2.;
    chat = Cgrav(AofH(hhat,p),p); 
    smin = chat +uhat;
    smax = chat +uhat; 
    for(j=0; j<2; j++){
        smin =  min3(um + pow(-1.,j)*Cgrav(q1m,Pm), uhat + pow(-1.,j)*chat,smin);
        smax =  max3(up + pow(-1.,j)*Cgrav(q1p,Pp),uhat+ pow(-1.,j)*chat,smax);
        }
    s[0] = smin;
    s[1] = smax;

   // printf("well shit. s is [%f,%f]\n", s[0],s[1]);
    if(isnan(s[0]) || isnan(s[1]))
    {
	    printf("well shit. s is [%f,%f]\n", s[0],s[1]);
	    printf("qm = %f, qp = %f, um = %f, up = %f,\n", q1m, q1p, up, um); 
    }
}



void Cpreiss::speedsHLL(double q1m, double q1p, double q2m, double q2p, double *s, bool Pm, bool Pp) //HLL speeds from Leon 2009 - they seem terrible...
{
    	double dry = 1e-6*At;;                                                  //pay attention to this!?
    	double cbar,Astar, ym,yp,cm, cp, um =0 , up= 0, ugh;	
	//double Astar1, Astar2;
	ym = HofA(q1m,Pm);
	yp = HofA(q1p,Pp);
	cm = Cgrav(q1m, Pm);
	cp = Cgrav(q1p, Pp);
	//if no dry bed present
	if(fmin(ym,yp)>=dry){
		cbar = (cp+cm)/2.;
		um = q2m/q1m;
		up = q2p/q1p;
		//cout<<"um = "<<um<< "up = " <<up<<endl;
	//	if (max(q1p,q1m)<0.7*Af){
			Astar = (q1m+q1p)/2.*(1+(um-up)/(PhiofA(q1p,Pp)+PhiofA(q1m,Pm)));//} //this verstion uses depth positivity condition
		//	printf("Astar = %f um = %f, up = %f intphim = %f intphip = %f\n",Astar, um, up, intPhi(q1m), intPhi(q1p));
		//	else{
				Astar = (q1m+q1p)/2.*(1+( (cbar>1e-6)? (um-up)/(2.*cbar): 0));//}  //this is linearized version
			if(Astar<0){Astar = (q1p+q1m)/2;}
		if (WTF)
			printf("Atsar = %f with q1m = %f and q1p = %f, um =%f, up = %f\n",Astar, cm, cp, um, up);
		//}
		//else{

		//}
		bool Ps = (Pm && Pp);
		s[0] = um - findOmega(Astar, q1m, Ps, Pm);
		s[1] = up + findOmega(Astar, q1p, Ps, Pp);
	}
	else{
		if(fmax(ym,yp)<dry)     // Both sides dry - both stay dry
		{
		//	printf("both sides dry\n");
		//	cout<<"P = ["<<Pm<<","<<Pp<<"]\n";
		//	cout<<"[ym, yp]=["<<ym<<","<<yp<<"]  "<<"[um, up]= ["<<um<<","<<up<<"]\n";
			s[0] = 0.;
			s[1] = 0.;
		}
		else if(ym<dry)  //left side dry
		{
		//	printf("left side dry\n");
		//	cout<<"P = ["<<Pm<<","<<Pp<<"]\n";
		//	cout<<"[ym, yp]=["<<ym<<","<<yp<<"]  "<<"[um, up]= ["<<um<<","<<up<<"]\n";

			up = q2p/q1p;
			s[0] = up - PhiofA(q1p,Pp);
			s[1] = up + cp;	
		}
		else if(yp<dry) //right side dry
		{
		//	printf("right side dry\n");
		//	cout<<"P = ["<<Pm<<","<<Pp<<"]\n";
		//	cout<<"[ym, yp]=["<<ym<<","<<yp<<"]  "<<"[um, up]= ["<<um<<","<<up<<"]\n";
			um = q2m/q1m;
			s[0] = um - cm;
			s[1] = um + PhiofA(q1m,Pm);
		}
		
	}
	if(isnan(s[0]) || isnan(s[1])) //check for NaNs
	{
		printf("Error!nan speeds!s[0] = %f, s[1] = %f, with y1 = %f, y2 = %f\n", s[0], s[1],ym, yp);
		printf("q1p is %f and q2p is %f, um is %f, up is %f\n", q1p, q2p, um, q2p);
		exit (1);
	}
	if(s[0]>s[1]) 	//check that HLL speeds have s0<s1; really, this should never bloody happen!
	{  
		printf("The hell? s[0]>s[1], with q1m = %f, q2m = %f, q1p =%f, q2p =%f, s[0] = %f, s[1] = %f\n",q1m,q2m,q1p,q2p,s[0],s[1]);
		double temp = s[1];
		s[1] = s[0];
		s[0] = temp;
	}
	cmax = max(cmax, max(fabs(s[0]),fabs(s[1]))); 
//	printf("as= [%f,%f], qs = [%f,%f], s=  [%f,%f]\n",q1m,q1p,q2m,q2p, s[0],s[1]);

		   
}


double Cpreiss::findOmega(double Astar, double Ak, bool Ps, bool Pk)
{
	double omega;
	double eps = 1e-8;
	//cout<<Astar<<"   "<< Ak<<endl;
	
    	if (Astar>Ak)
    	{
        	if(Astar<=Ak+eps)//if Ak-x is super small, the usual routine will fuck up
			{//so use Ak-eps small and taylor expand to evaluate Eta(Ak)-Eta(x) ~ dEta/dA(x)(x-Ak)
			//this is easy since c^2 = GdEta/dA, lol. 
				double c = Cgrav((Astar+Ak)/2.,Pk);
				return c*sqrt(Astar/Ak); 
		//		printf("c = %f\n", c);	
			}
		else{
			omega = sqrt((Eta(Astar, w, At, Ts, Ps)-Eta(Ak, w, At, Ts, Pk))*Astar/(Ak*(Astar-Ak)));
		}
	} 
	else
    	{
		//double y = hofA(Ak);
        	omega  = Cgrav(Ak,Ps);
    	}
	//cout<<"omega = "<<omega<<endl;	
	return omega;
}



/*
double Cpreiss::intPhiold(double A)
{	
	double Phi;	
	if (A<=At&&(!Pnow))
	{
		double t = thetaofA(A);
		Phi = powPhi(t,D);		
	}
	else
	{
		double t= thetaofA(At);
		Phi = powPhi(t,D)+2*sqrt(A/Ts)-2*sqrt(At/Ts);
	}
	return Phi;
}

double Cpreiss::Aofphiold(double phi)
{
	int count;
//	cout<<"phi ="<<phi<<endl;
	fphimlhs fp(D,At, Ts, phi);
//	cout<<"Here!? phi = "<<phi<<endl;
	double A= ::ridders(fp,0.,At*100.,&count, 1e-10, 1e-10);
	return A;
 
}
*/


////
//Junction definitions
////

Junction1::Junction1(Channel &a_ch0, int a_which, double a_bval, int a_bvaltype):ch0(a_ch0)
{
	N = ch0.N;
	bval = new double[ch0.M+1];
	for(int i=0;i<ch0.M+1; i++){bval[i] = a_bval;}

//	printf("bval[0] = %f\n", bval[0]);
	bvaltype = a_bvaltype;
	whichend = a_which;
	w = ch0.w;
	reflect = 0; //default is to use RI
}

/**apply boundary conditions at the end of a single pipe. Several options.
 * Computation controlled by these parameters:
 **** reflect = +1, -1, 0  (see below for details)
 **** whichend = 0, 1   (0 for left end, 1 for right--affects sign of things)  
 **** bvaltype = 0, 1   (0 for A, 1 for Q)  
 * Reflect: reflect all incoming waves (hard boundary), set reflect =1
 * Extrapolate: let all incoming waves through (ghost cell has same values as last interior cell), set reflect = -1
 * Specify: specify Q,A, or f(Q,A) = 0 (not implemented yet) 
 * following a characteristcic out of the domain to solve for unknown value, then updating boundary fluxes accordingly.
 * */

void Junction1::boundaryFluxes()
{
	double Ain, Qin, Aext, Qext;
	bool Pin, Pext;
	int pass = 0;
	double ctol = 0;//tolerance for trying to solve for characteristic solns
	double sign = pow(-1.,whichend+1);  //gives the sign in u (plus or minus) phi
	///if we're on the right side of pipe
	if (whichend)    
	{
		Ain = ch0.q[ch0.idx(0,N-1)];
		Qin = ch0.q[ch0.idx(1, N-1)];
		ch0.Pnow = ch0.P[N];
		Pin = ch0.P[N];
		Pext = ch0.P[N+1];
	}
	//if we're on the left side of the pipe
	else 
	{
		Ain = ch0.q[ch0.idx(0,0)];
		Qin = ch0.q[ch0.idx(1,0)];
		ch0.Pnow = ch0.P[1];
		Pin = ch0.P[1];
		Pext = ch0.P[0];
	}
	 //reflect =1 means reflection BC
	if(reflect ==1) 
	{
		Aext = Ain;
		Qext = -Qin;
	}

	//reflect =1 means extrapolation BC
	else if(reflect ==-1)
	{
		Aext = Ain;
		Qext = Qin;
	}
	//else we have a specified BC, eg Q(t) = z(t)
	else
	{
		//if specifying Q
		if(bvaltype==1) 
		{
			//sample the boundary time series at the current time step
			Qext = bval[ch0.n];	
			//if Qext != 0, problem is harder than if Q = 0 (this case treated seperately below)
			if(fabs(Qext)>0)
			{
				//first use Qext to find values c1min and c1max -- range of c+/_(A, Qext) over A in [0, \infty] 
				//if c1 = c+/_(Ain, Qin) lies in  [c1min, c1max], then it is feasible to follow a characteristic out of the doman
				double c1min=0., c1max=0., c1, xs=0.;
				double uin = (Ain>0. ?Qin/Ain :0. );
				//uniform cross section case is easy
				if(ch0.channeltype ==0)
				{
					c1 = uin +sign*2.*sqrt(G*Ain/w);
					c1min = 3.*pow((G*fabs(Qext)/w),1./3.); //min achievable value for for c+  (outgoing on right)
					c1max = -c1min;  			//max achievable value for c_ (outgoing on left)
				}
				//Preissman slot requires SUBTERFUDGE (formerly, rootfinding)
				else
				{	
					int count;
					c1  = uin +sign*ch0.PhiofA(Ain,Pin);
					if(sign<0)
					{	
						if(Qext<0)
						{
							//solve for xs s.t. 0 = Q + xs*c(xs).
						//	cout<<"Qext"<<Qext<<endl;
							fallpurpose fcpm(ch0.w, ch0.At, ch0.Ts, 0, Qext, 0,1,1., ch0.Pnow);
							xs = ::ridders(fcpm, 0,100,&count, 1e-10, 1e-10);	
							c1max = -Cgrav(xs, ch0.At, ch0.w, ch0.At, Pext)-PhiofA(xs, ch0.At, ch0.w,ch0.At, ch0.Pnow);
						}
					}
					else
					{
						if(Qext>0)
						{
							//solve for xs s.t. 0 = Q-xs*c(xs)
							
						//	cout<<"Qext"<<Qext<<endl;
							fallpurpose fcpm(ch0.w, ch0.At, ch0.Ts, 0, Qext,0, 1,-1., ch0.Pnow);
							xs = ::ridders(fcpm,0,100.,&count, 1e-10, 1e-10);	
							c1min = ch0.Cgrav(xs,ch0.Pnow)+ch0.PhiofA(xs,ch0.Pnow);
						}
					}	
				}
				//make sure left end boundary flux is enforceable 
				if(whichend ==0 && Qext<0. && c1>c1max-ctol)
				{
					printf("oops! Qext = %f is too small for c1 = %f\n, setting Aext =Ain= %f\n", Qext, c1,Ain);
					Aext = Ain;
					pass =1;
				/*	if(ch0.channeltype==0){bval[ch0.n] = w/G*pow(c1/3.,3.);}
					else{
						if(c1<=0)
						{
						//solve for x2 s.t. c1 = -c(x)-phi(x)
						int count;
						fallpurpose f2(ch0.w, ch0.At, ch0.Ts, c1, 0.,-1., 0.,-1., ch0.Pnow);
						double x2 = ::ridders(f2,0,100.,&count, 1e-10, 1e-8);	
						bval[ch0.n] = -x2*ch0.cgrav(ch0.hofA(x2));
						Aext = x2;
						//cout<<"c1 = "<<c1<<"Phi "<< ch0.intPhi(x2)<<" -c(x)-phi(x) = " <<-ch0.cgrav(ch0.hofA(x2)) -ch0.intPhi(x2)<<endl;
						}
						else{
							Qext = Qin;
							Aext = Ain;
							cout<<"This seems wrong but I'm confused..setting Qext = Qin\n";

						}
						pass =1;
					}
					Qext = bval[ch0.n];*/
					printf("Qext increased to min allowed value of %f, Aext = %f, RI = %f\n",Qext, Aext, Qext/Aext -ch0.PhiofA(Aext,Pext));
				
				}
				//make sure right end boundary flux is enforceable
				if(whichend ==1 && Qext>0. && c1<c1min+ctol)
				{
					printf("oops! Qext = %f is too large for c1 = %f\nsetting Aext =Ain=%f, Qin = %f\n", Qext,c1, Ain, Qin);
					pass = 1;
					Aext =Ain;
			/*		if(ch0.channeltype==0){bval[ch0.n] = w/G*pow(c1/3.,3.);}
					else{
						//solve for x2 s.t. c1 = c(x)+phi(x)
						int count;
						fallpurpose f2(ch0.w, ch0.At, ch0.Ts, c1, 0.,1., 0.,1., ch0.Pnow);
						double x2 = ::ridders(f2,0,100.,&count, 1e-10, 1e-8);	
						bval[ch0.n] = x2*ch0.cgrav(ch0.hofA(x2));
						Aext = x2;
						//cout<<"c1 = "<<c1<<"Phi "<< ch0.intPhi(x2)<<" -c(x)-phi(x) = " <<-ch0.cgrav(ch0.hofA(x2)) -ch0.intPhi(x2)<<endl;
						pass = 1;
					}
					Qext = bval[ch0.n];
					printf("Qext decreased to max allowed value of %f, Aext = %f\n",Qext, Aext);
					pass = 1;
				*/
				}
				
				if(ch0.channeltype ==0) //if uniform cross section
				{	
					double c2 = 2.*sign*sqrt(G/w);
					fRI f(c1,c2, bval[ch0.n]);
					dfRI df(c2,bval[ch0.n]);
					Aext = Newton(f,df,Ain, 1e-10, 100);
				}
				else if (ch0.Pnow)//pressurized R.I. are simple!
				{
					double c1t = c1 -sign*(ch0.PhiofA(ch0.At,false) -2*sqrt(G*ch0.At/ch0.Ts));
					double c2 = 2*sign*sqrt(G/ch0.Ts);
					fRI f(c1t,c2, bval[ch0.n]);
					dfRI df(c2,bval[ch0.n]);
					Aext = Newton(f,df,Ain, 1e-10, 100);

				}
				else 
				{
				//if flux was unenforceable, Aext was set above.
				if(!pass)
				{				
					int count;
					double uin = (Ain>0. ?Qin/Ain :0. );
					double lhs = uin +sign*ch0.PhiofA(Ain,Pin);
					//solve lhs = Qext/x +sign*phi(x) for x
					
				//	cout<<"Qext = "<<Qext<<"  lhs = "<<lhs<<"sign ="<<sign<<endl;
					fallpurpose fp(ch0.w, ch0.At,ch0.Ts, lhs, Qext, sign,1.,0., ch0.Pnow);
					Aext = ::ridders(fp,0.,ch0.Af*2,&count, 1e-10, 1e-10);
					double uext = (Aext>0 ?Qext/Aext :0.);
					double err = fabs(uext +sign*ch0.PhiofA(Aext,Pext)-lhs);
				//printf("ridders answer = %.16f, lhs = %f, Qext = %f, RI_ext-RI_n = %f\n", Aext,  lhs,Qext, err);
					}
				}
			}
			//if Qext = 0 we can solve it without rootfinding
			else{
				//uniform channel	
				if(ch0.channeltype ==0)
				{
					double uin = (Ain>0. ?Qin/Ain :0. );
					Aext = w/(G*4.)*pow((uin +sign*2.*sqrt(G*Ain/w)),2.);
				}
				//Preissman slot
				else
				{       			
					double lhs = Qin/Ain +sign*ch0.PhiofA(Ain,Pin);
					if(sign*lhs>=0)
					{
						Aext = ch0.AofPhi(sign*lhs,Pin);
					}
					else
					{
						cout<<"yeah I dunno, setting Aext =Ain"<<endl;
						printf("Qin = %f, Ain  %f, lhs= %f",Qin, Ain, lhs);
						Aext = Ain;
					}
					Qext = -Qin;
					Aext = Ain;
							
				}
			}
		//	printf("Ain is %f and Qin is %f and Aext-At is %f and Qext is %f for end %d\n", Ain, Qin, Aext-ch0.At, Qext, whichend);
		}

		//if we're specifying A
		else
		{
			Aext = bval[ch0.n];
			//Qext = (Qin/Ain+sign*Phi(Ain,ch0.w, ch0.At, ch0.Ts, Pin) - sign*Phi(Aext, ch0.w, ch0.At, ch0.Ts, Pext))*Aext;
			Qext = (Qin/Ain+sign*ch0.PhiofA(Ain,Pin) - sign*ch0.PhiofA(Aext,Pext))*Aext;
			//Qext = (Qin/Ain+sign*2.*sqrt(G*Ain/w) - sign*2.*sqrt(G*Aext/w))*Aext;
		//	printf("end %d has Qext is %f and Aext is %f and cgrav =%f\n", whichend, Qext, Aext, ch0.Cgrav(Aext, Pext));
		}
	}	
	//compute the fluxes using numFlux
	//right end of pipe
	if(whichend)
	{	
		ch0.numFlux(Ain, Aext, Qin, Qext, ch0.bfluxright, ch0.P[N], ch0.P[N+1]);
		ch0.q_hist[ch0.idx_t(0,N+1,ch0.n)] = Aext;
	//	cout<<"Aext = "<<Aext<<"  Qext = "<<Qext<<endl;
		ch0.q_hist[ch0.idx_t(1,N+1,ch0.n)] = Qext;
	//	printf("in junction routine!Aext =%f, Ain = %f, Qin %f, Qext = %f, bfluxright = [%f,%f]\n",Aext, Ain, Qin, Qext,ch0.bfluxright[0],ch0.bfluxright[1]);
		//update pressurization info--this needs work, I think... 
		if(reflect ==1||reflect==-1){ch0.P[N+1] =ch0.P[N];}
		else if(bvaltype==0 && Aext<ch0.At){ch0.P[N+1] = false;}
		else if(Aext>ch0.At){ch0.P[N+1]= true;}
		else{ch0.P[N+1] =ch0.P[N];}
	}
	//left end of pipe
	else
	{
		
		ch0.numFlux(Aext, Ain, Qext, Qin, ch0.bfluxleft, ch0.P[0], ch0.P[1]);
		//ch0.numFlux(Ain, Ain, Qin, Qin, ch0.bfluxleft, false, false);
		ch0.q_hist[ch0.idx_t(0,0,ch0.n)] = Aext;
		ch0.q_hist[ch0.idx_t(1,0,ch0.n)] = Qext;
		if(reflect ==-1||reflect ==1){ch0.P[0] =ch0.P[1];}
		else if(bvaltype==0 && Aext<ch0.At){ch0.P[0] = false;}
		else if(Aext>ch0.At){ch0.P[0]= true;}
		else{ch0.P[0] =ch0.P[1];}

	//	printf("in junction routine!Aext =%f, Ain = %f, Qin %f, Qext = %f, bfluxleft = [%f,%f]\n",Aext, Ain, Qin, Qext,ch0.bfluxleft[0],ch0.bfluxleft[1]);


	//	printf("Aext = %f and Qext = %f \n",Aext, Qext);
	//	cout<<ch0.bfluxleft[0]<<" "<<ch0.bfluxleft[1]<<endl;
	}
}

double Junction1::getFlowThrough(double dt)
{

	double Qt = 0;
	int N0;
	if (whichend)
	{
		N0 = ch0.N-1;	
	}
	else
	{
		N0 = 0;
	}
	for (int i = 0; i<ch0.M+1; i++)
	{	
		Qt+=dt*ch0.q_hist[ch0.idx_t(1,N0,i)];
	}
	return Qt;
}


/** overloaded functions to set boundary value time series*/
/** set boundary value to a constant value*/
void Junction1::setbVal(double bvalnew)
{
	for(int i =0; i<ch0.M+1; i++)
	{
		bval[i] = bvalnew;
	}
}

/**set boundary value to a time series stored in various arrays*/
void Junction1::setbVal(valarray<Real> x)
{
	for(int i =0; i<ch0.M+1; i++)
	{
		bval[i] = x[i];
	}
}

void Junction1::setbVal(vector<Real> x)
{
	for(int i =0; i<ch0.M+1; i++)
	{
		bval[i] = x[i];
	}
}


void Junction1::setbVal(double*x)
{
	for(int i =0; i<ch0.M+1; i++)
	{
		bval[i] = x[i];
	}
}

Junction1::~Junction1()
{
	delete [] bval;
}


Junction2::Junction2(Channel &a_ch0, Channel &a_ch1, int a_which0, int a_which1, double a_valveopen):ch0(a_ch0),ch1(a_ch1){
	whichend0 = a_which0;
	whichend1 = a_which1;
	if(whichend0){
		N0 = ch0.N-1;
		Ns0 = ch0.N+1;
	}
	else{
		N0 = 0;
		Ns0 = 0;
	}
	if(whichend1){
		N1 = ch1.N-1;
		Ns1 = ch1.N+1;
		
	}
	else{
		N1 = 0;
		Ns1= 0;
	}
	valvetimes.resize(ch0.M+1);
	valveopen = a_valveopen;
	valvetimes.resize(ch0.M+1);
	for(int i=0; i<ch0.M+1; i++)valvetimes[i]=valveopen;
	offset =0;	
}


/*fluxes across serial junction - only one reasonable way to do it:
*
*
*  ^ ~~~~~~~~                    channel 0 sees h1 -offset
*  |        | 			 channel 1 sees h0 +offset			
*  h0       |
*  |        |~~~~~~~~~~  ^
*  v _______|            |  h1    
*  ^        |            | 
* offset    |            |
*..v........|__________  v............ground   
*           | 
* channel 0 |channel 1
* */


void Junction2::setValveTimes(vector<Real>x)
{
	for (int i = 0; i<ch0.M+1; i++)
	{
		valvetimes[i]= x[i];
	}
}


void Junction2::setValveTimes(valarray<Real>x)
{
	for (int i = 0; i<ch0.M+1; i++)
	{
		valvetimes[i]= x[i];
	}
}

void Junction2::boundaryFluxes(){	
	double q1m, q1p, q2m, q2p, q1mfake, q1pfake;
	valveopen = valvetimes[ch0.n];
//	cout<<"valveopen="<<valveopen<<endl;
	q1m = ch0.q[ch0.idx(0,N0)];
	q2m = ch0.q[ch0.idx(1,N0)];
	q1p = ch1.q[ch1.idx(0,N1)];
	q2p = ch1.q[ch1.idx(1,N1)];
	bool pm = ch0.P[ch0.pj(N0)];
	bool pp = ch1.P[ch1.pj(N1)];
//attempt at incorporating valve opening coefficient - wish me luck/facepalm
	//printf("ws=  %f %f %f %f \n",w1[0], w1[1],w2[0], w2[1]);
	if(valveopen>0)
	{
		double h0f = ch1.HofA(q1p,pm)-offset;
		double h1f = ch0.HofA(q1m,pp)+offset;
		q1pfake = ch0.AofH(h0f,pm);      //what channel 0 sees
		q1mfake = ch1.AofH(h1f,pp);      //what channel 1 sees
	//	cout<< q1p-ch0.AofH(h0f,pm)<<" "<<q1m-ch1.AofH(h1f,pm)<<endl;
	//	printf("q1m = %f, q1mfake = %f, q1p = %f, q1pfake = %f, hm = %f, hp = %f N1 = %d N0 = %d\n", q1m, q1mfake,q1p,q1pfake, h0f, h1f, N1,N0);
	//	ch1.showVals(1);	
		if(whichend0)
		{
			ch0.numFlux(q1m,q1pfake, q2m, q2p*valveopen, ch0.bfluxright, ch0.P[Ns0], ch1.P[Ns1]);
			//ch1.numFlux(q1mfake, q1p, q2m*valveopen, q2p,  ch1.bfluxleft);
			ch1.bfluxleft[0] = ch0.bfluxright[0];	
			ch1.bfluxleft[1] = ch0.bfluxright[1];
			//printf("bfluxright = [%f,%f] and bfluxleft = [%f,%f]\n", ch0.bfluxright[0], ch0.bfluxright[1], ch1.bfluxleft[0], ch1.bfluxleft[1] ); 
		}
		else{
			ch0.numFlux(q1pfake, q1m, q2p*valveopen, q2m, ch0.bfluxleft, ch1.P[Ns1], ch0.P[Ns0]);
		//	ch1.numFlux(q1p, q1mfake, q2p, q2m*valveopen, ch1.bfluxright);
			ch1.bfluxright[0] = ch0.bfluxleft[0];
			ch1.bfluxright[1] = ch0.bfluxleft[1];
			//printf("bfluxleft = [%f,%f] and bfluxright = [%f,%f]\n", ch0.bfluxleft[0], ch0.bfluxleft[1], ch1.bfluxright[0], ch1.bfluxright[1] ); 
		}
		ch0.q_hist[ch0.idx_t(0,Ns0,ch0.n)] =  q1pfake;
		ch0.q_hist[ch0.idx_t(1,Ns0,ch0.n)] =  q2p*valveopen;
		ch1.q_hist[ch1.idx_t(0,Ns1,ch1.n)] =  q1mfake;
		ch1.q_hist[ch1.idx_t(1,Ns1,ch1.n)] =  q2m*valveopen;
		if(ch0.P[N0+1]==false)ch1.P[Ns1] =false;
		if(ch1.P[N1+1]==false)ch0.P[Ns0] =false;

   //    	printf("q1m =%f, q1mfake = %f, q1p = %f, q1pfake = %f, q2m = %f, q2p = %f\n", q1m, q1mfake, q1p, q1pfake, q2m, q2p); 
	     	
	}
	else //reflect to get 0 flux...
	{
		ch0.numFlux(q1m, q1m, q2m, -q2m, ch0.bfluxright, ch0.P[Ns0], ch1.P[Ns1]);
		ch1.numFlux(q1p, q1p, -q2p, q2p,  ch1.bfluxleft, ch0.P[Ns0], ch1.P[Ns1]);
	     	ch0.q_hist[ch0.idx_t(0,Ns0,ch0.n)] =  q1m;
		ch0.q_hist[ch0.idx_t(1,Ns0,ch0.n)] =  0.;
		ch1.q_hist[ch1.idx_t(0,Ns1,ch1.n)] =  q1p;
		ch1.q_hist[ch1.idx_t(1,Ns1,ch1.n)] =  0.;		
	}
}


Junction3::Junction3(Channel &ch0, Channel &ch1, Channel &ch2, int which0, int which1, int which2): 
			ch0(ch0), ch1(ch1), ch2(ch2),j2_01(ch0, ch1, which0, which1, 1), j2_12(ch1, ch2, which1, which2, 1),j2_21(ch2, ch1, which2, which1, 1), j2_20(ch0,ch2, which0, which2, 1)
{
	Ns[0] = ch0.N;
	Ns[1] = ch1.N;
	Ns[2] = ch2.N;
	whichend[0] = which0;
	whichend[1] = which1;
	whichend[2] = which2;

}

void Junction3::boundaryFluxes(){
	double flux0[2], flux1[2], flux2[2];
	double Abar[3], Qbar[3];
	double p01, p02, p10, p12, p20, p21;  //pik is the percentage of flux from pipe k going into pipe i; should have sum_k pik = 1 for each i...
	p01 = 0.5;
	p02 = 0.5;
	p10 = 1-p01;
	p12 = 0.5;
	p20 = 1-p02;
	p21 = 1-p12;
//	cout<<"junction 3 time godfuckingdamnallofthisfuckinguselessfuckingshit\n";
//this routine assumes you have one incoming (whichend =1) and two outgoing pipes (whichend =0)
//I can see needing to set up the machinery to have two incoming and one outgong but there's no reason to have anything else
	if((whichend[0]==1 &&whichend[1] ==0 &&whichend[2] ==0) || (whichend[0] ==0&& whichend[1] ==1 &&whichend[2] ==1))
	{

		//cout<<whichend[0]<<whichend[1]<<whichend[2]<<"!!!!!!"<<endl;
		Abar[0] = .5*(ch1.q[ch1.idx(0,0)]+ch2.q[ch2.idx(0,0)]);
		Qbar[0] = .5*(ch1.q[ch1.idx(1,0)]+ch2.q[ch2.idx(1,0)]);
		Abar[1] = .5*(ch0.q[ch0.idx(0,ch0.N-1)]+ch2.q[ch2.idx(0,0)]);
		Qbar[1] = .5*(ch0.q[ch0.idx(1,ch0.N-1)]-ch2.q[ch2.idx(1,0)]);
		Abar[2] = .5*(ch0.q[ch0.idx(0,ch0.N-1)]+ch1.q[ch1.idx(0,0)]);	
		Qbar[2] = .5*(ch0.q[ch0.idx(1,ch0.N-1)]-ch1.q[ch1.idx(1,0)]);
	//solve fake Reimann problems between each pair
	//0-1 is easy
		j2_01.boundaryFluxes();
		flux0[0] =  p01*ch0.bfluxright[0];
		flux0[1] =  p01*ch0.bfluxright[1];
		flux1[0] =  p10*ch1.bfluxleft[0];
		flux1[1] =  p10*ch1.bfluxleft[1];
	//1-2 is a pain in the neck	
		ch2.q[ch2.idx(1,0)]= -ch2.q[ch2.idx(1,0)];
	//	cout<<"ch1.q[0] ="<<ch1.q[ch1.idx(1,0)]<<endl; 
	//	cout<<"ch2.q[0] ="<<ch2.q[ch2.idx(1,0)]<<endl; 
		j2_12.boundaryFluxes();	
		ch2.q[ch2.idx(1,0)]= -ch2.q[ch2.idx(1,0)];
		flux1[0] += p12*ch1.bfluxleft[0];
		flux1[1] += p12*ch1.bfluxleft[1];
		flux2[0] =  p21*ch2.bfluxright[0];
		flux2[1] =  p21*ch2.bfluxright[1];

		ch1.q[ch1.idx(1,0)]= -ch1.q[ch1.idx(1,0)];
		j2_21.boundaryFluxes();
		ch1.q[ch1.idx(1,0)]= -ch1.q[ch1.idx(1,0)];
		flux2[0] = p21*ch2.bfluxleft[0];
		flux2[1] = p21*ch2.bfluxleft[1];
	

		j2_20.boundaryFluxes();
		flux0[0] += p02*ch0.bfluxright[0];
		flux0[1] += p02*ch0.bfluxright[1];
		flux2[0] += p20*ch2.bfluxleft[0];
		flux2[1] += p20*ch2.bfluxleft[1];

	//	for(int i=0; i<2;i++)
		//set all the fluxes properly
		ch0.bfluxright[0] = flux0[0];
		ch0.bfluxright[1] = flux0[1];
		ch1.bfluxleft[0] = flux1[0];
		ch1.bfluxleft[1] = flux1[1];
		ch2.bfluxleft[0] = flux2[0];
		ch2.bfluxleft[1] = flux2[1];
	//store away the info
	ch0.q_hist[ch0.idx_t(0,ch0.N+1,ch0.n+1)] = Abar[0]; 
	ch0.q_hist[ch0.idx_t(1,ch0.N+1,ch0.n+1)] = Qbar[0];
	ch1.q_hist[ch1.idx_t(0,0,ch1.n)] =  Abar[1];
	ch1.q_hist[ch1.idx_t(1,0,ch1.n)] =  Qbar[1];
	ch2.q_hist[ch2.idx_t(0,0,ch2.n)] =  Abar[2];
	ch2.q_hist[ch2.idx_t(1,0,ch2.n)] =  Qbar[2];
//	printf("flux0 = [%f, %f], flux 1 = [%f, %f], flux2 = [%f, %f]\n", flux0[0], flux0[1], flux1[0], flux1[1], flux2[0], flux2[1]);
	}
	else{	cout<<"End 0 = "<<whichend[0]<<" End 1 = "<< whichend[1]<<" End 2 = "<<whichend[2]<<endl;
		cout<<"Have not implemented triple junctions for this configuration!Sorry!\n";}





}
