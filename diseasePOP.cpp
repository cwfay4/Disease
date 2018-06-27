#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "random.hpp"
#include "graph.hpp"
#include "diseasePOP.hpp"

using namespace std;


dPOP::dPOP () {
   n_immune =0;
   n_sick=0;
   n_healthy =0;
   n_dead =0;
   n_nc=0;
   p_Immune =0;
   p_sick =0;
   contagin =0;
   fatality =0;
   I=false;
   randomvectors=false;
   lifetime=100;   
   return;
}
dPOP::dPOP (int a, int b, int c, int d, double e, double f, double g, double h) {  //I don't know if I am using this
   n_immune =a;
   n_sick=b;
   n_healthy =c;
   n_dead =d;
   p_Immune =e;
   p_sick =f;
   contagin =g;
   fatality =h;   
   lifetime=100; 
   I=false;
   randomvectors=false;
   return;
}
void dPOP::clear(){
   n_immune =0;
   n_sick=0;
   n_healthy =0;
   n_dead =0;
   n_nc=0;
   return;	
}
void dPOP::randomize_node_list(int n, std::vector<int>& nlist, long &idum){
   std::vector<int> nlist2(nlist.size(),0);
   int j;
   double dj;
   bool debug=false;
  
   for(unsigned int i=0; i<nlist.size(); i++){
      dj=fmod(ran2(&idum)*nlist.size(),nlist.size()); 
      j=(int) dj; // chose a random node from list 2 and place it in i in the list;	   
      std::iter_swap(nlist.begin()+i,nlist.begin()+j);
   }

   if (debug) cout<<" nlist size after randomizing "<<nlist.size()<<endl;
   nlist2.clear();
   return;
}
void dPOP::precondition(graph& g, long &seed){
   long idum=seed;
   bool screen=false;
     
   clear();
     
   for (unsigned int i=1;i<=1000;i++){ //initialize random number generator
      ran2(&seed);
   }   
      
   for(int i=0; i<g.get_n(); i++){
	  g.nodes[i].stateP=0;	   
      if (g.nodes[i].edges.size()==0){ //if node has no edges set to uncovered (0)
         g.nodes[i].frz=true;
	     g.nodes[i].intState=-1; //isolated node ignore it
	     n_nc++;  //if its isolated (no edges) add one to the n_nc counter
      }
      else if (ran2(&idum)<p_Immune){  // healthy immune
	     g.nodes[i].intState=0;
	     g.nodes[i].frz=true;
	     n_immune++;
	  }
	  else if (ran2(&idum)<p_sick){ //sick
		 g.nodes[i].intState=1;
	     g.nodes[i].frz=false; 
	     n_sick++;
      }
      else {// healthy not immune
		 g.nodes[i].frz=false;
	     g.nodes[i].intState=0;
	     n_healthy++;
      }
      if (g.get_debug())cout<<i<<" state"<<g.nodes[i].intState<<" frz"<<g.nodes[i].frz<<endl;
   }
   if (screen) cout<<" sick "<<n_sick<<" Healthy "<<n_healthy<<" Immune "<<n_immune<<" dead "<<n_dead<<"non-connected"<<n_nc<<endl;
//   g.calc_sum_P();
   return;   
}
void dPOP::pop_evolve(graph& g, long seed2){
	bool conv=true;
	bool debug=false;
	long idum=seed2;
	
	double dn = (double) g.get_n();
	
	std::vector<int> nlist(g.get_n(),0);
	for(unsigned int i=0; i<nlist.size(); i++){
      nlist[i]=i;
    }
    
	randomize_node_list(g.get_n(), nlist, seed2);
	
	//if (g.get_debug()) {
	//	cout<<a.n_sick<<" "<<a.n_healthy<<" "<<a.n_immune;
	//    cout<<"node list randomized ";
	// 	for(unsigned int i=0; i<nlist.size(); i++){
    //      cout<<" nlist "<<nlist[i]<<" ";
    //    } 
    //    cout<<endl;  
	// }
    n_sick_max=n_sick;
    n_sick_mIter=0;
    for (unsigned it=0; it<POP_STEPS; it++){ //iterate over POP_STEPS = time
		randomize_node_list(g.get_n(), nlist, seed2);
		for(unsigned i=0; i<nlist.size(); i++){ //for all nodes
			if(debug) cout<<i<<" state"<<g.nodes[i].intState<<" frz"<<g.nodes[i].frz<<endl;
			if (!g.nodes[i].frz && g.nodes[i].intState==1){ //if you are sick but not dead
			   if(debug) cout<<" you are sick but not dead"<<endl;
			   if (g.nodes[i].stateP > lifetime && ran2(&idum)<p_Recovery){//adds the stipulation of a probability of recovery instead of just a lifetime

				   if(ran2(&idum) < fatality || (simple && !I)){ //oops you died (could also be considered recovered/immune)
					  g.nodes[i].frz =true;
					  n_dead++;
					  n_sick--;
					 if(g.get_debug()) cout<<" you died!"<<endl;
				   }
				   else {  //bam! you are healed
					  g.nodes[i].intState=0;
					  if (I) {  //...and immune.
	                    g.nodes[i].frz=true; 
	                    n_immune++;
	                  }
	                  else n_healthy++;
	                  n_sick--;
	                  g.nodes[i].stateP=0;
	                  if(debug) cout<<" you are healed!"<<endl;
			       }
			   }
			   else g.nodes[i].stateP++; //time counter for sickness
			}			
			else if (!g.nodes[i].frz && g.nodes[i].intState==0){ //if you are healthy, you might get sick.
			  if(debug) cout<<" you are healthy"<<endl;
			   double n_I=0; //Is this the number of edges or interactions?
			   for (int j=0; j < g.nodes[i].edges.size(); j++){//for all edges connedtd to i
	               int n2=g.nodes[i].edges[j];
	               if (g.nodes[n2].intState==1 && !g.nodes[n2].frz) n_I++; //a node that is frozen and sick is dead!
	           }
	           double prob = 1.0 - pow((1.0-contagin),n_I); //probability of getting sick
	           if(debug) cout<<prob<<" ";
	           if (ran2(&idum)<prob){ //Are you sick
                  g.nodes[i].intState=1; //apparently yes.
	              g.nodes[i].frz=false;
	              g.nodes[i].stateP=0;
	              n_sick++;
	              n_healthy--;
	              if(debug) cout<<" You are now sick"<<endl;
		       }
	           else if (!simple && ran2(&idum)<p_sick){  //areyou randomly getting sick?
				  g.nodes[i].intState=1; //apparently yes.
	              g.nodes[i].frz=false;
	              g.nodes[i].stateP=0;
	              n_sick++;
	              n_healthy--; 
			   }
		    }
		}
	  
      if (conv){ 
	     std::ofstream output("Disease_Conv.csv", ios_base::app);
         output<<it<<", "<<(double)n_healthy/(dn-n_nc)<<", "<<(double)n_sick/(dn-n_nc)<<", "<<(double)n_immune/(dn-n_nc)<<", "<<(double)n_dead/(dn-n_nc)<<endl;
	     output.close();
	     
      }
     // if (g.get_debug()) cout<<n_healthy<<" "<<n_sick<<" "<<n_immune<<endl;
      if (n_sick_max<n_sick){
		 n_sick_max=n_sick;  //peak of the number of infected
         n_sick_mIter=it;     //time step of peak of infected
      }
   }
//  if(g.get_debug())  cout<<" sick "<<n_sick<<" Healthy "<<n_healthy<<" Immune "<<n_immune<<" dead "<<n_dead<<endl;
  if(g.get_debug())  cout<<" sick "<<(double)n_sick/(dn-n_nc)<<" Healthy "<<(double)n_healthy/(dn-n_nc)<<" Immune "<<(double)n_immune/(dn-n_nc)<<" dead "<<(double)n_dead/(dn-n_nc)<<endl;
   return;	
}
	
