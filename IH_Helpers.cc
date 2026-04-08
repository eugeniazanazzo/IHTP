// File IH_Helpers.hh
#include "IH_Helpers.hh"
#include<cmath>

IH_SolutionManager::IH_SolutionManager(const IH_Input & pin) 
  : SolutionManager<IH_Input,IH_Output,DefaultCostStructure<int>>(pin, "IHSolutionManager")  {} 

void IH_SolutionManager::RandomState(IH_Output& out) 
{
  int p,i,s,r,selected,cd,cr,cot,cn;
  out.Reset();
  for(p=0;p<in.Patients();p++){
    if(in.PatientMandatory(p)){
      do{
        cd=in.PatientPossibleAdmission(p,Random::Uniform<int>(0,in.PatientNumPossibleAdmission(p)-1));
        cot=Random::Uniform<int>(0,in.OperatingTheaters()-1);
      }while(in.OperatingTheaterAvailability(cot,cd)==0);
      do{
        cr=Random::Uniform<int>(0,in.Rooms()-1);
      }while(in.IncompatibleRoom(p,cr) || in.OccupantsPresent(cr,cd)==in.RoomCapacity(cr));
      out.AssignPatient(p,cd,cr,cot);
    }
  }
  selected=Random::Uniform<int>(0,in.OptionalPatients()-1); 
  out.SetScheduled(selected);
  for(i=0;i<selected;i++){
    do{
      p=Random::Uniform<int>(0,in.Patients()-1);
    }while(in.PatientMandatory(p) || out.ScheduledPatient(p));
    do{
      cd=in.PatientPossibleAdmission(p,Random::Uniform<int>(0,in.PatientNumPossibleAdmission(p)-1));
      cot=Random::Uniform<int>(0,in.OperatingTheaters()-1);
    }while(in.OperatingTheaterAvailability(cot,cd)==0);
    do{
      cr=Random::Uniform<int>(0,in.Rooms()-1);
    }while(in.IncompatibleRoom(p,cr));
    out.AssignPatient(p,cd,cr,cot);
  }
  out.UpdatewithOccupantsInfo();

  for(s=0;s<in.Shifts();s++){
    for(r=0;r<in.Rooms();r++){
        cn=in.AvailableNurse(s,Random::Uniform<int>(0,in.AvailableNurses(s)-1));
        out.AssignNurse(cn,r,s);
    }
  }
}

bool IH_SolutionManager::CheckConsistency(const IH_Output& out) const
{
  for (int i=0;i<in.Rooms();i++){
    for(int d=0;d<in.Days();d++){
      if(out.RoomDayLoad(i,d)>in.RoomCapacity(i))
        return false;
      if(out.RoomDayAPatients(i,d)>0 && out.RoomDayBPatients(i,d)>0)
        return false;
    }
  }
  for(int d=0;d<in.Days();d++){
    for (int o=0;o<in.OperatingTheaters();o++){
      if(out.OperatingTheaterDayLoad(o,d)>in.OperatingTheaterAvailability(o,d))
        return false;
    }
    for (int s=0;s<in.Surgeons();s++){
      if(out.SurgeonDayLoad(s,d)>in.SurgeonMaxSurgeryTime(s,d))
        return false;
    }
  }
  for(int p=0;p<in.Patients();p++){
    if(out.ScheduledPatient(p)){
      if(out.AdmissionDay(p)<in.PatientSurgeryReleaseDay(p) || out.AdmissionDay(p)>in.PatientLastPossibleDay(p))
        return false;
      if(in.IncompatibleRoom(p,out.Room(p)))
        return false;
    }
  }
  return true; 
}

void IH_SolutionManager::PrettyPrintOutput(const IH_Output& out, string filename) const
{
  ofstream os(filename);
  out.PrettyPrint(os,"");
  if (!CheckConsistency(out))
  {
    throw runtime_error("Output is not consistent");
  }
  os.close();
}

//Hard constraint 1
int IH_OvertimeSurgeon::ComputeCost(const IH_Output& out) const
{
  int cost = 0;
  for(int s=0;s<in.Surgeons();s++){
    for(int d=0;d<in.Days();d++){
      if(in.SurgeonMaxSurgeryTime(s,d)<out.SurgeonDayLoad(s,d))
        cost+=out.SurgeonDayLoad(s,d)-in.SurgeonMaxSurgeryTime(s,d);
    }
  }
  return cost;
}

void IH_OvertimeSurgeon::PrintViolations(const IH_Output& out, ostream& os) const{
  for(int s=0;s<in.Surgeons();s++){
    for(int d=0;d<in.Days();d++){
      if(in.SurgeonMaxSurgeryTime(s,d)<out.SurgeonDayLoad(s,d))
        os<<" On day "<<d<<" surgeon "<<s<<" works for "<<out.SurgeonDayLoad(s,d)<<" minutes but should work "<<in.SurgeonMaxSurgeryTime(s,d)<<endl;
    }
  }
}

//Hard constraint  2
int IH_OvertimeOT::ComputeCost(const IH_Output& out) const
{
  int cost = 0;
  for(int t=0;t<in.OperatingTheaters();t++){
    for(int d=0;d<in.Days();d++){
      if(out.OperatingTheaterDayLoad(t,d)>in.OperatingTheaterAvailability(t,d))
        cost+=out.OperatingTheaterDayLoad(t,d)-in.OperatingTheaterAvailability(t,d);
    }
  }
  return cost;
}

void IH_OvertimeOT::PrintViolations(const IH_Output& out, ostream& os) const{
  for(int t=0;t<in.OperatingTheaters();t++){
    for(int d=0;d<in.Days();d++){
      if(out.OperatingTheaterDayLoad(t,d)>in.OperatingTheaterAvailability(t,d))
        os<<" On day "<<d<<" operating room "<<t<<" is open for "<<out.OperatingTheaterDayLoad(t,d)<<" while should be open for "<<in.OperatingTheaterAvailability(t,d)<<endl;
    }
  }
}

//Hard constraint 3
int IH_OverCapacity::ComputeCost(const IH_Output& out) const
{
  int cost = 0;
  for(int r=0;r<in.Rooms();r++){
    for(int d=0;d<in.Days();d++){
      if(out.RoomDayLoad(r,d)>in.RoomCapacity(r))
        cost+=out.RoomDayLoad(r,d)-in.RoomCapacity(r);
    }
  }
  return cost;
}

void IH_OverCapacity::PrintViolations(const IH_Output& out, ostream& os) const{
for(int r=0;r<in.Rooms();r++){
    for(int d=0;d<in.Days();d++){
      if(out.RoomDayLoad(r,d)>in.RoomCapacity(r))
        os<<"Room "<<r<<" exceeds its capacity by "<<out.RoomDayLoad(r,d)-in.RoomCapacity(r)<<" during day "<<d<<endl;
    }
  }
}

//Hard constraint 4
int IH_GenderMix::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0;
  for(int r=0;r<in.Rooms();r++){
    for(int d=0;d<in.Days();d++){
      if(out.RoomDayBPatients(r,d)>0 && out.RoomDayAPatients(r,d)>0)
        cost+=min(out.RoomDayBPatients(r,d),out.RoomDayAPatients(r,d));
    }
  }
  return cost;
}

void IH_GenderMix::PrintViolations(const IH_Output& out, ostream& os) const
{
  for(int r=0;r<in.Rooms();r++){
    for(int d=0;d<in.Days();d++){
      if(out.RoomDayBPatients(r,d)>0 && out.RoomDayAPatients(r,d)>0)
        os<<"Room "<<r<<" on day "<<d<<" has a balance of "<<out.RoomDayBPatients(r,d)<<" B "<<out.RoomDayAPatients(r,d)<<" A "<<endl;
    }
  }
}


int IH_Unscheduled::ComputeCost(const IH_Output& out) const
{ 
  return in.OptionalPatients()-out.Scheduled();
}

void IH_Unscheduled::PrintViolations(const IH_Output& out, ostream& os) const{
  os<<"there are "<<in.OptionalPatients()-out.Scheduled()<<" unscheduled (optional) patients "<<endl;
}

//Soft constraint 2
int IH_Delay::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0;
  for(int p=0;p<in.Patients();p++){
    if(out.ScheduledPatient(p))
      cost+=out.AdmissionDay(p)-in.PatientSurgeryReleaseDay(p);
  }
  return cost;
}

void IH_Delay::PrintViolations(const IH_Output& out, ostream& os) const{
  for(int p=0;p<in.Patients();p++){
    if(out.ScheduledPatient(p)){
      if(in.PatientSurgeryReleaseDay(p)!=out.AdmissionDay(p))
        os<<"Patient "<<p<<" should have been admitted on day "<<in.PatientSurgeryReleaseDay(p)<<" but it's actually admitted on day "<<out.AdmissionDay(p)<<endl;
    }
  }
}

int IH_OpenOT::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0;
  for(int d=0;d<in.Days();d++){
    for(int t=0;t<in.OperatingTheaters();t++){
      if(out.OperatingTheaterDayLoad(t,d)>0)
        cost++;
    }
  }
  return cost;
}

void IH_OpenOT::PrintViolations(const IH_Output& out,ostream& os) const
{ 
  int open;
  for(int d=0;d<in.Days();d++){
    open=0;
    for(int t=0;t<in.OperatingTheaters();t++){
      if(out.OperatingTheaterDayLoad(t,d)>0)
        open++;
    }
    os<<"On day "<<d<<" there are "<<open<<" open OT"<<endl;
  }
}

int IH_AgeMix::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0,min,max,p;
  for(int d=0;d<in.Days();d++){
    for(int r=0;r<in.Rooms();r++){
      min=-1,max=-1;
      for(int i=0;i<out.RoomDayLoad(r,d);i++){
        p=out.RoomDayPatient(r,d,i);
        if(p<in.Patients()){
          if(min==-1){
            min=in.PatientAgeGroup(p);
            max=min;
          }
          else if(in.PatientAgeGroup(p)>max)
            max=in.PatientAgeGroup(p);
          else if(in.PatientAgeGroup(p)<min)
            min=in.PatientAgeGroup(p);
        }
        else{
          if(min==-1){
            min=in.OccupantAgeGroup(p-in.Patients());
            max=min;
          }
          else if(in.OccupantAgeGroup(p-in.Patients())>max)
            max=in.OccupantAgeGroup(p-in.Patients());
          else if(in.OccupantAgeGroup(p-in.Patients())<min)
            min=in.OccupantAgeGroup(p-in.Patients());
        }
      }
      cost+=max-min;
    }
  }
  return cost;
}

void IH_AgeMix::PrintViolations(const IH_Output& out, ostream& os) const
{
  int min,max,p;
  for(int d=0;d<in.Days();d++){
    for(int r=0;r<in.Rooms();r++){
      min=-1,max=-1;
      for(int i=0;i<out.RoomDayLoad(r,d);i++){
        p=out.RoomDayPatient(r,d,i);
        if(p<in.Patients()){
          if(min==-1){
            min=in.PatientAgeGroup(p);
            max=min;
          }
          else if(in.PatientAgeGroup(p)>max)
            max=in.PatientAgeGroup(p);
          else if(in.PatientAgeGroup(p)<min)
            min=in.PatientAgeGroup(p);
        }
        else{
          if(min==-1){
            min=in.OccupantAgeGroup(p-in.Patients());
            max=min;
          }
          else if(in.OccupantAgeGroup(p-in.Patients())>max)
            max=in.OccupantAgeGroup(p-in.Patients());
          else if(in.OccupantAgeGroup(p-in.Patients())<min)
            min=in.OccupantAgeGroup(p-in.Patients());
        }
      }
      if(max-min>0)
        os<<"In Room "<<r<<" during day "<<d<<" the age bounds are "<<min<<"--"<<max<<endl;
    }
  }
}

int IH_Skill::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0,cn,p,cs;
  for(int r=0;r<in.Rooms();r++){
    for(int s=0;s<in.Shifts();s++){
      if(out.RoomDayLoad(r,s/in.ShiftsPerDay())>0){
        cn=out.NurseAttending(r,s);
        for(int i=0;i<out.RoomDayLoad(r,s/in.ShiftsPerDay());i++){
          p=out.RoomDayPatient(r,s/in.ShiftsPerDay(),i);
          if(p<in.Patients()){
            cs=s-(out.AdmissionDay(p)*in.ShiftsPerDay());
            if(in.PatientSkillLevelRequired(p,cs)>in.NurseSkillLevel(cn))
              cost+=in.PatientSkillLevelRequired(p,cs)-in.NurseSkillLevel(cn);
          }
          else{
            if(in.OccupantSkillLevelRequired(p-in.Patients(),s)>in.NurseSkillLevel(cn))
              cost+=in.OccupantSkillLevelRequired(p-in.Patients(),s)-in.NurseSkillLevel(cn);
          }
        }
      }
    }
  }
  return cost;
}

void IH_Skill::PrintViolations(const IH_Output& out, ostream& os) const{
  int cn,p,cs;
  for(int r=0;r<in.Rooms();r++){
    for(int s=0;s<in.Shifts();s++){
      if(out.RoomDayLoad(r,s/in.ShiftsPerDay())>0){
        cn=out.NurseAttending(r,s);
        for(int i=0;i<out.RoomDayLoad(r,s/in.ShiftsPerDay());i++){
        p=out.RoomDayPatient(r,s/in.ShiftsPerDay(),i);
        if(p<in.Patients()){
            cs=s-(out.AdmissionDay(p)*in.ShiftsPerDay());
            if(in.PatientSkillLevelRequired(p,cs)>in.NurseSkillLevel(cn))
              os<<"During shift "<<s<<" there is a skill there is a deficit between the skill of nurse "<<cn<<"("<<in.NurseSkillLevel(cn)<<")"<<" and level required by patient "<<p<<"("<<in.PatientSkillLevelRequired(p,cs)<<")"<<endl;
          }
          else{
            if(in.OccupantSkillLevelRequired(p-in.Patients(),s)>in.NurseSkillLevel(cn))
             os<<"During shift "<<s<<" there is a skill there is a deficit between the skill of nurse "<<cn<<"("<<in.NurseSkillLevel(cn)<<")"<<" and level required by patient fixed "<<p<<"("<<in.OccupantSkillLevelRequired(p-in.Patients(),s)<<")"<<endl;
          }
        }
      }
    }
  }
}
int IH_Excess::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0,cs;
  for(int n=0;n<in.Nurses();n++){
    for(int s=0;s<in.NurseWorkingShifts(n);s++){
      cs=in.NurseWorkingShift(n,s);
      if(in.NurseMaxLoad(n,cs)<out.NurseShiftLoad(n,cs))
        cost+=out.NurseShiftLoad(n,cs)-in.NurseMaxLoad(n,cs);
    }
  }
  return cost;
}

void IH_Excess::PrintViolations(const IH_Output& out, ostream& os) const{
  int cs;
  for(int n=0;n<in.Nurses();n++){
    for(int s=0;s<in.NurseWorkingShifts(n);s++){
      cs=in.NurseWorkingShift(n,s);
      if(in.NurseMaxLoad(n,cs)<out.NurseShiftLoad(n,cs))
        os<<"During shift "<<cs<<" nurse "<<n<<" has a workload of "<<out.NurseShiftLoad(n,cs)<< " over the maximum load "<<in.NurseMaxLoad(n,cs)<<endl;
    }
  }
}

int IH_Continuity::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0, count;
  for (int o = 0; o < in.Occupants(); o++)
  {
    count = out.CountOccupantNurses(o);
    if (count > 0)
      cost += count;
  }
  for (int p = 0; p < in.Patients(); p++)
  {
    if (out.AdmissionDay(p) != -1)
    {
      count = out.CountDistinctNurses(p);
      if (count > 0)
        cost += count;
    }
  }
  return cost;
}

void IH_Continuity::PrintViolations(const IH_Output& out, ostream& os) const
{
  int count;
  for (int o = 0; o < in.Occupants(); o++)
  {
    count = out.CountOccupantNurses(o);
    if (count > 0)
     os << count << " distinct nurses for patient " << in.OccupantId(o-in.Patients()) << endl;
  }
  for (int p = 0; p < in.Patients(); p++)
  {
    if (out.AdmissionDay(p) != -1)
    {
      count = out.CountDistinctNurses(p);
      if (count > 0)
        os << count << " distinct nurses for patient " << in.PatientId(p) << endl;
    }
  }
}

int IH_SurgeonTransfer::ComputeCost(const IH_Output& out) const
{ 
  int cost = 0;
  for(int s=0;s<in.Surgeons();s++){
    for(int d=0;d<in.Days();d++){
      if(out.SurgeonDayTheaters(s,d)>1)
        cost+=out.SurgeonDayTheaters(s,d)-1;
    }
  }
  return cost;
}

void IH_SurgeonTransfer::PrintViolations(const IH_Output& out, ostream& os) const{
  for(int s=0;s<in.Surgeons();s++)
    for(int d=0;d<in.Days();d++)
      if(out.SurgeonDayTheaters(s,d)>1)
        os<<"During day "<<d<<" surgeon "<<s<<" visits "<<out.SurgeonDayTheaters(s,d)<<" OT "<<endl;
}

//Moves

bool operator==(const IH_MoveAdmission& mv1, const IH_MoveAdmission& mv2)
{
  return (mv1.patient==mv2.patient &&
    mv1.new_ad==mv2.new_ad &&
    mv1.new_r==mv2.new_r &&
    mv1.new_ot==mv2.new_ot);
}

bool operator!=(const IH_MoveAdmission& mv1, const IH_MoveAdmission& mv2)
{
  return (mv1.patient!=mv2.patient ||
    mv1.new_ad!=mv2.new_ad ||
    mv1.new_r!=mv2.new_r ||
    mv1.new_ot!=mv2.new_ot );
}

bool operator<(const IH_MoveAdmission& mv1, const IH_MoveAdmission& mv2)
{
  return (mv1.patient < mv2.patient)
    || (mv1.patient == mv2.patient && mv1.new_ad < mv2.new_ad)
    || (mv1.patient == mv2.patient && mv1.new_ad == mv2.new_ad && mv1.new_r<mv2.new_r)
    || (mv1.patient == mv2.patient && mv1.new_ad == mv2.new_ad && mv1.new_r==mv2.new_r && mv1.new_ot<mv2.new_ot);
}

istream& operator>>(istream& is, IH_MoveAdmission& mv)
{
  char ch;
  is>>mv.patient>>ch>>mv.new_ad>>ch>>mv.new_r>>ch>>mv.new_ot;
	return is;
}

ostream& operator<<(ostream& os, const IH_MoveAdmission& mv)
{
  os<<"MA "<<mv.patient<<","<<mv.new_ad<<","<<mv.new_r<<","<<mv.new_ot<<endl;
  return os;
}

void IH_MoveAdmissionNeighborhoodExplorer::RandomMove(const IH_Output& out, IH_MoveAdmission& mv) const
{
  do{
    mv.patient=Random::Uniform<int>(0,in.Patients()-1);
  }while(in.PatientMandatory(mv.patient) && in.PatientNumPossibleAdmission(mv.patient)==1);
  bool mandatory;
  int i,oad=out.AdmissionDay(mv.patient);
  if(oad==-1)
    mandatory=true;
  else 
    mandatory=in.PatientMandatory(mv.patient)?true:false;
  do{
    i=Random::Uniform<int>(0,in.PatientNumPossibleAdmission(mv.patient)-mandatory);
  } while(i<in.PatientNumPossibleAdmission(mv.patient) && in.PatientPossibleAdmission(mv.patient,i)==oad);
  if(i==in.PatientNumPossibleAdmission(mv.patient)){ 
    mv.new_ad=-1;
    mv.new_ot=-1;
    mv.new_r=-1;
  }
  else{
    mv.new_ad=in.PatientPossibleAdmission(mv.patient,i);
    do
    {
      mv.new_ot=Random::Uniform<int>(0,in.OperatingTheaters()-1);
    } while (in.OperatingTheaterAvailability(mv.new_ot,mv.new_ad)<in.PatientSurgeryDuration(mv.patient));
    int timeout=in.Rooms();
    do{
      mv.new_r=Random::Uniform<int>(0,in.Rooms()-1);
      timeout--;
    }while(timeout>0 && (in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,mv.new_ad)==in.RoomCapacity(mv.new_r)));
    if(timeout<=0){
      throw EmptyNeighborhood();
    }
  }
} 

bool IH_MoveAdmissionNeighborhoodExplorer::FeasibleMove(const IH_Output&, const IH_MoveAdmission& mv) const
{
  return (!(in.PatientMandatory(mv.patient)) || mv.new_ad!=-1) &&
        (mv.new_ad==-1 || in.OperatingTheaterAvailability(mv.new_ot,mv.new_ad)>in.PatientSurgeryDuration(mv.patient));
} 

void IH_MoveAdmissionNeighborhoodExplorer::MakeMove(IH_Output& out, const IH_MoveAdmission& mv) const
{
  out.MoveAdmission(mv.patient,mv.new_ad,mv.new_r,mv.new_ot);
}  

void IH_MoveAdmissionNeighborhoodExplorer::FirstMove(const IH_Output& out, IH_MoveAdmission& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
  AnyNextMove(out,mv);	
}


void IH_MoveAdmissionNeighborhoodExplorer::AnyFirstMove(const IH_Output& out, IH_MoveAdmission& mv) const
{
  mv.patient=0;
  while(in.PatientMandatory(mv.patient) && in.PatientNumPossibleAdmission(mv.patient)==1){
    mv.patient++;
  }
  mv.ind=-1;
  mv.new_r=-1;
  if(out.AdmissionDay(mv.patient)!=-1 && !in.PatientMandatory(mv.patient)){
    mv.new_ad=-1;
    mv.new_ot=-1;
  }
  else{
    do{
      mv.ind++;
    }while(mv.ind<in.PatientNumPossibleAdmission(mv.patient) && in.PatientPossibleAdmission(mv.patient,mv.ind)==out.AdmissionDay(mv.patient));
    mv.new_ad=in.PatientPossibleAdmission(mv.patient,mv.ind);
    do{
      mv.new_r++;
    }while(in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,mv.new_ad)==in.RoomCapacity(mv.new_r));
    mv.new_ot=0;
  }
}

bool IH_MoveAdmissionNeighborhoodExplorer::NextMove(const IH_Output& out, IH_MoveAdmission& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IH_MoveAdmissionNeighborhoodExplorer::AnyNextMove(const IH_Output& out, IH_MoveAdmission& mv) const
{
  if (NextOT(out,mv)) 
    return true;
  else if (NextRoom(out,mv)) 
  {
    if(mv.new_ad==-1)
      mv.new_ot=-1;
    else
      mv.new_ot=0; 
    return true;
  }
  else if (NextAdmissionDate(out,mv))
  {
    FirstRoom(out,mv);
    if(mv.new_ad==-1)
      mv.new_ot=-1;
    else
      mv.new_ot=0;
    return true;
  }
  else if (mv.patient < in.Patients() - 1) 
  {
    do{
      mv.patient++;
    } while(in.PatientNumPossibleAdmission(mv.patient)==1 && in.PatientMandatory(mv.patient));
    if(mv.patient==in.Patients())
      return false;
    else{  
      FirstAdmissionDate(out,mv);
    if(mv.new_ad==-1)
      mv.new_ot=-1;
    else
      mv.new_ot=0;
    return true;
    }
  }
  return false;
}

bool IH_MoveAdmissionNeighborhoodExplorer::NextOT(const IH_Output&, IH_MoveAdmission& mv) const{
  mv.new_ot++;
  return (mv.new_ad!=-1 && mv.new_ot<in.OperatingTheaters());
}

bool IH_MoveAdmissionNeighborhoodExplorer::NextRoom(const IH_Output&, IH_MoveAdmission& mv) const{
  do{
  mv.new_r++;
  }while(mv.new_r<in.Rooms() && (in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,mv.new_ad)==in.RoomCapacity(mv.new_r)));
  return (mv.new_ad!=-1 && mv.new_r<in.Rooms());
}

bool IH_MoveAdmissionNeighborhoodExplorer::NextAdmissionDate(const IH_Output& out, IH_MoveAdmission& mv) const{
  do{
  mv.ind++;
  }while(mv.ind<in.PatientNumPossibleAdmission(mv.patient) && in.PatientPossibleAdmission(mv.patient,mv.ind)==out.AdmissionDay(mv.patient));
  if(mv.ind<in.PatientNumPossibleAdmission(mv.patient))
    mv.new_ad=in.PatientPossibleAdmission(mv.patient,mv.ind);
  return (mv.ind<in.PatientNumPossibleAdmission(mv.patient));
}

void IH_MoveAdmissionNeighborhoodExplorer::FirstAdmissionDate(const IH_Output& out, IH_MoveAdmission& mv) const{
  
  if(!in.PatientMandatory(mv.patient) && out.AdmissionDay(mv.patient)!=-1){
    mv.new_ad=-1;
    mv.new_r=-1;
    mv.new_ot=-1;
    mv.ind=-1;
  }
  else{
    mv.ind=0;
    while(in.PatientPossibleAdmission(mv.patient,mv.ind)==out.AdmissionDay(mv.patient))
      mv.ind++;
    mv.new_ad=in.PatientPossibleAdmission(mv.patient,mv.ind);
    FirstRoom(out,mv);
  }
}

void IH_MoveAdmissionNeighborhoodExplorer::FirstRoom(const IH_Output&, IH_MoveAdmission& mv) const{
  
  mv.new_r=0;
  while(in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,mv.new_ad)==in.RoomCapacity(mv.new_r))
    mv.new_r++;
}

//Change OT

bool operator==(const IH_ChangeOT& mv1, const IH_ChangeOT& mv2)
{
  return (mv1.patient==mv2.patient &&
    mv1.new_ot==mv2.new_ot);
}

bool operator!=(const IH_ChangeOT& mv1, const IH_ChangeOT& mv2)
{
  return (mv1.patient!=mv2.patient ||
    mv1.new_ot!=mv2.new_ot );
}

bool operator<(const IH_ChangeOT& mv1, const IH_ChangeOT& mv2)
{
  return (mv1.patient < mv2.patient)
    || (mv1.patient == mv2.patient && mv1.new_ot<mv2.new_ot);
}

istream& operator>>(istream& is, IH_ChangeOT& mv)
{
  char ch;
  is>>mv.patient>>ch>>mv.new_ot;
	return is;
}

ostream& operator<<(ostream& os, const IH_ChangeOT& mv)
{
  os<<"COT "<<"<"<<mv.patient<<","<<mv.new_ot<<">"<<endl;
  return os;
}

void IH_ChangeOTNeighborhoodExplorer::RandomMove(const IH_Output& out, IH_ChangeOT& mv) const{
  
  int timeout=in.OperatingTheaters();
  do{
    mv.patient=Random::Uniform<int>(0,in.Patients()-1);
    mv.new_ot=Random::Uniform<int>(0,in.OperatingTheaters()-1);
    timeout--;
  }while(timeout> 0 && (mv.new_ot==out.OT(mv.patient) || out.AdmissionDay(mv.patient)==-1 || in.OperatingTheaterAvailability(mv.new_ot,out.AdmissionDay(mv.patient))<in.PatientSurgeryDuration(mv.patient)));
  if(timeout<=0){
    throw EmptyNeighborhood();
  }
} 

bool IH_ChangeOTNeighborhoodExplorer::FeasibleMove(const IH_Output& out, const IH_ChangeOT& mv) const
{
  return (in.OperatingTheaterAvailability(mv.new_ot,out.AdmissionDay(mv.patient))>in.PatientSurgeryDuration(mv.patient));
} 

void IH_ChangeOTNeighborhoodExplorer::MakeMove(IH_Output& out, const IH_ChangeOT& mv) const
{ 
  out.ChangeOT(mv.patient,mv.new_ot);
} 

void IH_ChangeOTNeighborhoodExplorer::FirstMove(const IH_Output& out, IH_ChangeOT& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
  AnyNextMove(out,mv);	
}

void IH_ChangeOTNeighborhoodExplorer::AnyFirstMove(const IH_Output& out, IH_ChangeOT& mv) const
{
  do{
    mv.patient++;
  }while(out.AdmissionDay(mv.patient)==-1);
  do{
    mv.new_ot++;
  }while(out.OT(mv.patient)==mv.new_ot);
}

bool IH_ChangeOTNeighborhoodExplorer::NextMove(const IH_Output& out, IH_ChangeOT& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IH_ChangeOTNeighborhoodExplorer::AnyNextMove(const IH_Output& out, IH_ChangeOT& mv) const
{
  if (NextOT(out,mv)) 
  {
    return true;
  }
  else if (mv.patient < in.Patients() - 1) 
  {
    do{
      mv.patient++;
    } while(out.AdmissionDay(mv.patient)==-1);
    if(mv.patient>=in.Patients())
      return false;
    else{  
      FirstOT(out,mv);
      return true;
    }
  }
return false;
}

bool IH_ChangeOTNeighborhoodExplorer::NextOT(const IH_Output& out, IH_ChangeOT& mv) const{
  do{
  mv.new_ot++;
  } while(mv.new_ot==out.OT(mv.patient));
  return (mv.new_ot<in.OperatingTheaters());
}

void IH_ChangeOTNeighborhoodExplorer::FirstOT(const IH_Output& out, IH_ChangeOT& mv) const{
  mv.new_ot=0;
  while(mv.new_ot==out.OT(mv.patient))
    mv.new_ot++;
}

//ChangeRoom 

bool operator==(const IH_ChangeRoom& mv1, const IH_ChangeRoom& mv2)
{
  return (mv1.patient==mv2.patient &&
    mv1.new_r==mv2.new_r);
}

bool operator!=(const IH_ChangeRoom& mv1, const IH_ChangeRoom& mv2)
{
  return (mv1.patient!=mv2.patient ||
    mv1.new_r!=mv2.new_r );
}

bool operator<(const IH_ChangeRoom& mv1, const IH_ChangeRoom& mv2)
{
  return (mv1.patient < mv2.patient)
    || (mv1.patient == mv2.patient && mv1.new_r<mv2.new_r);
}

istream& operator>>(istream& is, IH_ChangeRoom& mv)
{
  char ch;
  is>>mv.patient>>ch>>mv.new_r;
	return is;
}

ostream& operator<<(ostream& os, const IH_ChangeRoom& mv)
{
  os<<"CR "<<mv.patient<<","<<mv.new_r<<endl;
  return os;
}

void IH_ChangeRoomNeighborhoodExplorer::RandomMove(const IH_Output& out, IH_ChangeRoom& mv) const{
  
  int timeout=in.Rooms();
  do{
    mv.patient=Random::Uniform<int>(0,in.Patients()-1);
  }while(out.AdmissionDay(mv.patient)==-1);
  do{
  mv.new_r=Random::Uniform<int>(0,in.Rooms()-1);
  timeout--;
  }while(timeout>0 && (mv.new_r==out.Room(mv.patient) || in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,out.AdmissionDay(mv.patient))==in.RoomCapacity(mv.new_r)));
  if(timeout<=0){
    throw EmptyNeighborhood();
  }
} 

bool IH_ChangeRoomNeighborhoodExplorer::FeasibleMove(const IH_Output&, const IH_ChangeRoom& mv) const
{
  return mv.new_r<in.Rooms();
}

void IH_ChangeRoomNeighborhoodExplorer::MakeMove(IH_Output& out, const IH_ChangeRoom& mv) const
{ 
  out.ChangeRoom(mv.patient,mv.new_r);
} 

void IH_ChangeRoomNeighborhoodExplorer::FirstMove(const IH_Output& out, IH_ChangeRoom& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
  AnyNextMove(out,mv);	
}

void IH_ChangeRoomNeighborhoodExplorer::AnyFirstMove(const IH_Output& out, IH_ChangeRoom& mv) const
{
  do{
    mv.patient++;
  }while(out.AdmissionDay(mv.patient)==-1);
  do{
    mv.new_r++;
  }while(mv.new_r<in.Rooms() && (out.Room(mv.patient)==mv.new_r || in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,out.AdmissionDay(mv.patient))==in.RoomCapacity(mv.new_r)));
}

bool IH_ChangeRoomNeighborhoodExplorer::NextMove(const IH_Output& out, IH_ChangeRoom& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IH_ChangeRoomNeighborhoodExplorer::AnyNextMove(const IH_Output& out, IH_ChangeRoom& mv) const
{
  if (NextRoom(out,mv)) 
  {
    return true;
  }
  else if (mv.patient < in.Patients() - 1) 
  {
    do{
      mv.patient++;
    } while(out.AdmissionDay(mv.patient)==-1);
    if(mv.patient>=in.Patients())
      return false;
    else{  
      FirstRoom(out,mv);
      return true;
    }
  }
return false;
}

bool IH_ChangeRoomNeighborhoodExplorer::NextRoom(const IH_Output& out, IH_ChangeRoom& mv) const{
  do{
  mv.new_r++;
  } while(mv.new_r<in.Rooms() && (mv.new_r==out.Room(mv.patient)|| in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,out.AdmissionDay(mv.patient))==in.RoomCapacity(mv.new_r)));
  return (mv.new_r<in.Rooms());
}

void IH_ChangeRoomNeighborhoodExplorer::FirstRoom(const IH_Output& out, IH_ChangeRoom& mv) const{
  mv.new_r=0;
  while(mv.new_r<in.Rooms() && (mv.new_r==out.Room(mv.patient) || in.IncompatibleRoom(mv.patient,mv.new_r) || in.OccupantsPresent(mv.new_r,out.AdmissionDay(mv.patient))==in.RoomCapacity(mv.new_r)))
    mv.new_r++;
}

//Change Nurse

bool operator==(const IH_ChangeNurse& mv1, const IH_ChangeNurse& mv2)
{
  return (mv1.sh==mv2.sh &&
    mv1.r==mv2.r &&
    mv1.nn==mv2.nn);
}

bool operator!=(const IH_ChangeNurse& mv1, const IH_ChangeNurse& mv2)
{
  return (mv1.sh!=mv2.sh ||
    mv1.r!=mv2.r ||
    mv1.nn==mv2.nn);
}

bool operator<(const IH_ChangeNurse& mv1, const IH_ChangeNurse& mv2)
{
  return (mv1.sh<mv2.sh) ||
    (mv1.sh==mv2.sh && mv1.r<mv2.r) ||
    (mv1.sh==mv2.sh && mv1.r==mv2.r && mv1.nn<mv2.nn);
}

istream& operator>>(istream& is, IH_ChangeNurse& mv)
{
  char ch;
  is>>mv.sh>>ch>>mv.r>>ch>>mv.nn;
	return is;
}

ostream& operator<<(ostream& os, const IH_ChangeNurse& mv)
{
  os<<"CN "<<"("<<mv.sh<<","<<mv.r<<","<<mv.nn<<")"<<endl;
  return os;
}


void IH_ChangeNurseNeighborhoodExplorer::RandomMove(const IH_Output& out, IH_ChangeNurse& mv) const{
  
  do{
  mv.sh=Random::Uniform<int>(0,in.Shifts()-1);
  mv.r=Random::Uniform<int>(0,in.Rooms()-1);
  }while(out.RoomDayLoad(mv.r,mv.sh/in.ShiftsPerDay())==0 || in.AvailableNurses(mv.sh)==1);
  do{
    mv.idx=Random::Uniform<int>(0,in.AvailableNurses(mv.sh)-1);
    mv.nn=in.AvailableNurse(mv.sh,mv.idx);
  }while(mv.nn==out.NurseAttending(mv.r,mv.sh));
} 

void IH_ChangeNurseNeighborhoodExplorer::MakeMove(IH_Output& out, const IH_ChangeNurse& mv) const{
  out.ChangeNurse(mv.sh,mv.r,mv.nn);
}

bool IH_ChangeNurseNeighborhoodExplorer::FeasibleMove(const IH_Output&, const IH_ChangeNurse& mv) const
{
  return mv.r<in.Rooms() && in.NurseMaxLoad(mv.nn,mv.sh)>0;
}

void IH_ChangeNurseNeighborhoodExplorer::FirstMove(const IH_Output& out, IH_ChangeNurse& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
    AnyNextMove(out,mv);	
}

void IH_ChangeNurseNeighborhoodExplorer::AnyFirstMove(const IH_Output& out, IH_ChangeNurse& mv) const
{
  mv.sh=0;
  do{
    mv.r++;
  }while(out.RoomDayLoad(mv.r,mv.sh/in.ShiftsPerDay())==0);
  if(mv.r<in.Rooms()){
    do{
      mv.idx++;
      mv.nn=in.AvailableNurse(mv.sh,mv.idx);
    }while(mv.nn==out.NurseAttending(mv.r,mv.sh));
  }
}

bool IH_ChangeNurseNeighborhoodExplorer::NextMove(const IH_Output& out, IH_ChangeNurse& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IH_ChangeNurseNeighborhoodExplorer::AnyNextMove(const IH_Output& out, IH_ChangeNurse& mv) const
{
  if (NextNurse(out,mv)) 
  {
    return true;
  }
  else if(NextRoom(out,mv)){
    FirstNurse(out,mv);
    return true;
  }
  else if (mv.sh < in.Shifts() - 1) 
  {
    FirstSRNCombo(out,mv);
    if(mv.sh>=in.Shifts())
      return false;
    return true;
  }
return false;
}

void IH_ChangeNurseNeighborhoodExplorer::FirstNurse(const IH_Output& out, IH_ChangeNurse& mv) const{
  mv.idx=0;
  while(mv.r<in.Rooms() && in.AvailableNurse(mv.sh,mv.idx)==out.NurseAttending(mv.r,mv.sh))
    mv.idx++;
  if(mv.r<in.Rooms() && mv.idx<in.AvailableNurses(mv.sh))
    mv.nn=in.AvailableNurse(mv.sh,mv.idx);
}

bool IH_ChangeNurseNeighborhoodExplorer::NextNurse(const IH_Output& out, IH_ChangeNurse& mv) const{
  if(mv.r==in.Rooms())
    return false;
  do{
  mv.idx++;
  } while(mv.idx<in.AvailableNurses(mv.sh) && in.AvailableNurse(mv.sh,mv.idx)==out.NurseAttending(mv.r,mv.sh));
  if(mv.idx<in.AvailableNurses(mv.sh))
    mv.nn=in.AvailableNurse(mv.sh,mv.idx);
  return mv.idx<in.AvailableNurses(mv.sh);
}

void IH_ChangeNurseNeighborhoodExplorer::FirstRoom(const IH_Output& out, IH_ChangeNurse& mv) const{
  mv.r=0;
  if(mv.sh<in.Shifts()){
    while(mv.r<in.Rooms() && out.RoomDayLoad(mv.r,(mv.sh/in.ShiftsPerDay()))==0)
      mv.r++;
    if(mv.r<in.Rooms())
      FirstNurse(out,mv);
  }
}

void IH_ChangeNurseNeighborhoodExplorer::FirstSRNCombo(const IH_Output& out, IH_ChangeNurse& mv) const{
  do{
    mv.sh++;
    FirstRoom(out,mv);
  }while(mv.sh<in.Rooms() && (mv.r>=in.Rooms() || mv.idx>=in.AvailableNurses(mv.sh)));
}

bool IH_ChangeNurseNeighborhoodExplorer::NextRoom(const IH_Output& out, IH_ChangeNurse& mv) const{
  do{
  mv.r++;
  } while(mv.r<in.Rooms() && out.RoomDayLoad(mv.r,(mv.sh/in.ShiftsPerDay()))==0);
  return mv.r<in.Rooms();
}


//Swap Patients

bool operator==(const IH_SwapPatients& mv1, const IH_SwapPatients& mv2)
{
  return ((mv1.p1==mv2.p1) && (mv1.p2==mv2.p2)) || ((mv1.p1==mv2.p2) && (mv1.p2==mv2.p1));
}

bool operator!=(const IH_SwapPatients& mv1, const IH_SwapPatients& mv2)
{
  return ((mv1.p1!=mv2.p1) || (mv1.p2!=mv2.p2)) && ((mv1.p1!=mv2.p2) || (mv1.p2!=mv2.p1));
}
bool operator <(const IH_SwapPatients& mv1, const IH_SwapPatients& mv2)
{
  return (mv1.p1<mv2.p1) || (mv1.p1==mv2.p1 && mv1.p2<mv2.p2);
}

istream& operator>>(istream& is, IH_SwapPatients& mv)
{
  char ch;
  is>>mv.p1>>ch>>mv.day1>>ch>>mv.p2>>ch>>mv.day2;
  return is;
}

ostream& operator<<(ostream& os, const IH_SwapPatients& mv)
{
  os<<"SP "<<"("<<mv.p1<<","<<mv.day1<<","<<mv.p2<<","<<mv.day2<<")"<<endl;
  return os;
}

void IH_SwapPatientsNeighborhoodExplorer::RandomMove(const IH_Output& out, IH_SwapPatients& mv) const{
  do{
    int index,loop=0;
    do{
      mv.p1=Random::Uniform<int>(0,in.Patients()-1);
      mv.day1=out.AdmissionDay(mv.p1);
    }while(out.AdmissionDay(mv.p1)==-1 || in.PossibleSwaps(mv.p1,mv.day1)==0);
    do{
      index=Random::Uniform<int>(0,in.PossibleSwaps(mv.p1,mv.day1)-1);
      mv.p2=in.SwappablePatient(mv.p1,mv.day1,index);
      mv.day2=out.AdmissionDay(mv.p2);
      loop++;
    }while(loop<in.PossibleSwaps(mv.p1,mv.day1) && !out.Admissable(mv.p1,mv.day2));
  }while(!FeasibleMove(out,mv));
  if(mv.day1>mv.day2){
    swap(mv.day1,mv.day2);
    swap(mv.p1,mv.p2);
  }
}

bool IH_SwapPatientsNeighborhoodExplorer::FeasibleMove(const IH_Output& out, const IH_SwapPatients& mv) const
{
  return (mv.p1!=mv.p2) &&
         (mv.day1!=-1 || !in.PatientMandatory(mv.p2)) && (mv.day2!=-1 || !in.PatientMandatory(mv.p1)) &&
         (mv.day1==-1 || !in.IncompatibleRoom(mv.p2,out.Room(mv.p1))) && (mv.day2==-1 || !in.IncompatibleRoom(mv.p1,out.Room(mv.p2))) &&
         (mv.day1==-1 || in.PatientAdmissable(mv.p2,mv.day1)) && (mv.day2==-1 || in.PatientAdmissable(mv.p1,mv.day2)); 
}

void IH_SwapPatientsNeighborhoodExplorer::MakeMove(IH_Output& out, const IH_SwapPatients& mv) const
{
  out.SwapPatients(mv.p1,mv.p2);
}

void IH_SwapPatientsNeighborhoodExplorer::FirstMove(const IH_Output& out, IH_SwapPatients& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
    AnyNextMove(out,mv);	
}

void IH_SwapPatientsNeighborhoodExplorer::AnyFirstMove(const IH_Output& out, IH_SwapPatients& mv) const
{
  mv.p1=0;
  mv.p2=0;
  mv.day1=out.AdmissionDay(mv.p1);
  do{
    mv.p2++;
    mv.day2=out.AdmissionDay(mv.p2);
  } while((mv.day1==-1 && mv.day2==-1));
}

bool IH_SwapPatientsNeighborhoodExplorer::NextMove(const IH_Output& out, IH_SwapPatients& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IH_SwapPatientsNeighborhoodExplorer::AnyNextMove(const IH_Output& out, IH_SwapPatients& mv) const
{
  if (mv.p2<in.Patients()-1) 
  {
    FindNextP2(out,mv);
    if(mv.p1==in.Patients()-1)
      return false;
    else return true;
  }
  else if (mv.p1<in.Patients()-2) 
  {
    mv.p1++;
    mv.day1=out.AdmissionDay(mv.p1);
    mv.p2=mv.p1;
    FindNextP2(out,mv);
    if(mv.p1<in.Patients()-1)
      return true;
    else return false;
  }
  return false;
}

void IH_SwapPatientsNeighborhoodExplorer::FindNextP2(const IH_Output& out, IH_SwapPatients& mv) const{
  do{
    mv.p2++;
    mv.day2=out.AdmissionDay(mv.p2);
  }while(mv.p2<=in.Patients()-1 && (mv.day1==-1 && mv.day2==-1));
  if(mv.p2>in.Patients()-1 || (mv.p2==in.Patients()-1 && mv.day1==-1 && mv.day2==-1))
    FindNextPair(out,mv);

}

void IH_SwapPatientsNeighborhoodExplorer::FindNextPair(const IH_Output& out, IH_SwapPatients& mv) const{
  mv.p1++;
  mv.day1=out.AdmissionDay(mv.p1);
  mv.p2=mv.p1;
  do{
    mv.p2++;
    mv.day2=out.AdmissionDay(mv.p2);
  }while(mv.p2<=in.Patients()-1 && mv.day1==-1 && mv.day2==-1);
  if(mv.p1<in.Patients()-1 && mv.p2==in.Patients()-1 && mv.day1==-1 && mv.day2==-1)
    FindNextPair(out,mv);
}

//Deltas

int IH_MoveAdmissionDeltaOvertimeSurgeon::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  return out.DeltaSurgeonOvertimeMP(mv.patient,mv.new_ad);
}

int IH_MoveAdmissionDeltaOvertimeOT::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0;
  int oad=out.AdmissionDay(mv.patient),oot=out.OT(mv.patient),d=in.PatientSurgeryDuration(mv.patient);
  if(oad!=-1 && out.OperatingTheaterDayLoad(oot,oad)>in.OperatingTheaterAvailability(oot,oad)){
    if(out.OperatingTheaterDayLoad(oot,oad)-d>in.OperatingTheaterAvailability(oot,oad))
      delta-=d;
    else
      delta-=out.OperatingTheaterDayLoad(oot,oad)-in.OperatingTheaterAvailability(oot,oad);
  }
  if(mv.new_ad!=-1 && out.OperatingTheaterDayLoad(mv.new_ot,mv.new_ad)+d>in.OperatingTheaterAvailability(mv.new_ot,mv.new_ad)){
    if(out.OperatingTheaterDayLoad(mv.new_ot,mv.new_ad)>in.OperatingTheaterAvailability(mv.new_ot,mv.new_ad))
      delta+=d;
    else
      delta+=out.OperatingTheaterDayLoad(mv.new_ot,mv.new_ad)+d-in.OperatingTheaterAvailability(mv.new_ot,mv.new_ad);
  }
  return delta;
}

int IH_MoveAdmissionDeltaOverCapacity::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),nad=mv.new_ad,oad=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient);
  if(old_r!=mv.new_r || !in.Overlap(mv.patient,oad,nad)){
    for(i=0;i<min(st,in.Days()-oad);i++){
      if(old_r!=-1 && out.RoomDayLoad(old_r,oad+i)>in.RoomCapacity(old_r))
        delta--;
    }
    for(i=0;i<min(st,in.Days()-nad);i++){
      if(mv.new_r!=-1 && out.RoomDayLoad(mv.new_r,mv.new_ad+i)>=in.RoomCapacity(mv.new_r))
        delta++;
    }
  }
  else{ 
    if(oad<nad){ 
      for(i=oad;i<nad;i++){
        if(out.RoomDayLoad(old_r,i)>in.RoomCapacity(old_r))
          delta--;
      }
      for(i=oad+st;i<min(nad+st,in.Days());i++){
        if(out.RoomDayLoad(mv.new_r,i)>=in.RoomCapacity(mv.new_r))
          delta++;
      }
    }
    else{
      for(i=nad;i<oad;i++){
        if(out.RoomDayLoad(mv.new_r,i)>=in.RoomCapacity(mv.new_r))
          delta++;
      }
      for(i=nad+st;i<min(oad+st,in.Days());i++){
        if(out.RoomDayLoad(old_r,i)>in.RoomCapacity(old_r))
          delta--;
      }
    }
  }
  return delta;
}

int IH_MoveAdmissionDeltaGenderMix::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),nad=mv.new_ad,oad=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient);
  if(old_r!=mv.new_r || !in.Overlap(mv.patient,oad,nad)){
    for(i=0;i<min(st,in.Days()-oad);i++){
      if(old_r!=-1 && out.RoomDayBPatients(old_r,oad+i)>0 && out.RoomDayAPatients(old_r,oad+i)>0){
        if(out.RoomDayBPatients(old_r,oad+i)>=out.RoomDayAPatients(old_r,oad+i) && in.PatientGender(mv.patient)==Gender::A)
          delta--;
        else if(out.RoomDayBPatients(old_r,oad+i)<=out.RoomDayAPatients(old_r,oad+i) && in.PatientGender(mv.patient)==Gender::B)
          delta--;
      }
    }
    for(i=0;i<min(st,in.Days()-nad);i++){
      if(mv.new_r!=-1 && (out.RoomDayBPatients(mv.new_r,nad+i)>0 || out.RoomDayAPatients(mv.new_r,nad+i)>0)){
        if(out.RoomDayBPatients(mv.new_r,nad+i)>out.RoomDayAPatients(mv.new_r,nad+i) && in.PatientGender(mv.patient)==Gender::A)
          delta++;
        else if(out.RoomDayBPatients(mv.new_r,nad+i)<out.RoomDayAPatients(mv.new_r,nad+i) && in.PatientGender(mv.patient)==Gender::B)
          delta++;
      }
    }
  }
  else{ 
    if(oad<nad){ 
      for(i=oad;i<nad;i++){
        if(old_r!=-1 && out.RoomDayBPatients(old_r,i)>0 && out.RoomDayAPatients(old_r,i)>0){
          if(out.RoomDayBPatients(old_r,i)>=out.RoomDayAPatients(old_r,i) && in.PatientGender(mv.patient)==Gender::A)
            delta--;
          else if(out.RoomDayBPatients(old_r,i)<=out.RoomDayAPatients(old_r,i) && in.PatientGender(mv.patient)==Gender::B)
            delta--;
        }
      }
      for(i=oad+st;i<min(nad+st,in.Days());i++){
        if(mv.new_r!=-1 && (out.RoomDayBPatients(mv.new_r,i)>0 || out.RoomDayAPatients(mv.new_r,i)>0)){
          if(out.RoomDayBPatients(mv.new_r,i)>out.RoomDayAPatients(mv.new_r,i) && in.PatientGender(mv.patient)==Gender::A)
            delta++;
          else if(out.RoomDayBPatients(mv.new_r,i)<out.RoomDayAPatients(mv.new_r,i) && in.PatientGender(mv.patient)==Gender::B)
            delta++;
        }
      }
    }
    else{
      for(i=nad;i<oad;i++){
        if(mv.new_r!=-1 && (out.RoomDayBPatients(mv.new_r,i)>0 || out.RoomDayAPatients(mv.new_r,i)>0)){
          if(out.RoomDayBPatients(mv.new_r,i)>out.RoomDayAPatients(mv.new_r,i) && in.PatientGender(mv.patient)==Gender::A)
            delta++;
          else if(out.RoomDayBPatients(mv.new_r,i)<out.RoomDayAPatients(mv.new_r,i) && in.PatientGender(mv.patient)==Gender::B)
            delta++;
        }
      }
      for(i=nad+st;i<min(oad+st,in.Days());i++){
        if(old_r!=-1 && out.RoomDayBPatients(old_r,i)>0 && out.RoomDayAPatients(old_r,i)>0){
          if(out.RoomDayBPatients(old_r,i)>=out.RoomDayAPatients(old_r,i) && in.PatientGender(mv.patient)==Gender::A)
            delta--;
          else if(out.RoomDayBPatients(old_r,i)<=out.RoomDayAPatients(old_r,i) && in.PatientGender(mv.patient)==Gender::B)
            delta--;
        }
      }
    }
  }
  return delta;
}

int IH_MoveAdmissionDeltaUnscheduled::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  if(out.AdmissionDay(mv.patient)==-1)
    return -1;
  if(mv.new_ad==-1)
    return 1;
  else return 0;
}

int IH_MoveAdmissionDeltaDelay::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0;
  int oad=out.AdmissionDay(mv.patient),rd=in.PatientSurgeryReleaseDay(mv.patient);
  if(oad==-1) 
    delta+=mv.new_ad-rd;
  else if(mv.new_ad==-1)
    delta-=oad-rd;
  else
    delta += mv.new_ad - oad;
  return delta;
} 

int IH_MoveAdmissionDeltaOpenOT::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0;
  int oad=out.AdmissionDay(mv.patient),o_ot=out.OT(mv.patient),d=in.PatientSurgeryDuration(mv.patient);
  if(oad!=-1 && out.OperatingTheaterDayLoad(o_ot,oad)==d)
    delta--;
  if(mv.new_ad!=-1 && out.OperatingTheaterDayLoad(mv.new_ot,mv.new_ad)==0)
    delta++; 
  return delta;
} 

int IH_MoveAdmissionDeltaAgeMix::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),nad=mv.new_ad,oad=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),age=in.PatientAgeGroup(mv.patient);
  if(old_r!=mv.new_r || !in.Overlap(mv.patient,oad,nad)){ 
    for(i=0;i<min(st,in.Days()-oad);i++){ 
      if(old_r!=-1 && out.RoomDayMin(old_r,oad+i)!=-1 && age==out.RoomDayMin(old_r,oad+i) && out.RoomAgePres(old_r,oad+i,age)==1 && out.NewProjMin(old_r,oad+i)!=-1)
        delta-=out.NewProjMin(old_r,oad+i)-age;
      else if(old_r!=-1 && out.RoomDayMax(old_r,oad+i)!=-1 && age==out.RoomDayMax(old_r,oad+i) && out.RoomAgePres(old_r,oad+i,age)==1 && out.NewProjMax(old_r,oad+i)!=-1){
        delta-=age-out.NewProjMax(old_r,oad+i);
      }
    }
    for(i=0;i<min(st,in.Days()-nad);i++){ 
      if(mv.new_r!=-1 && out.RoomDayMin(mv.new_r,nad+i)>age)
        delta+=out.RoomDayMin(mv.new_r,nad+i)-age;
      if(mv.new_r!=-1 && out.RoomDayMax(mv.new_r,nad+i)!=-1 && out.RoomDayMax(mv.new_r,nad+i)<age)
        delta+=age-out.RoomDayMax(mv.new_r,nad+i);
    }
  }
  else{ 
    if(oad<nad){ 
      for(i=oad;i<nad;i++){
        if(out.RoomDayMin(old_r,i)!=-1 && age==out.RoomDayMin(old_r,i) && out.RoomAgePres(old_r,i,age)==1 && out.NewProjMin(old_r,i)!=-1)
          delta-=out.NewProjMin(old_r,i)-age;
        else if(out.RoomDayMax(old_r,i)!=-1 && age==out.RoomDayMax(old_r,i) && out.RoomAgePres(old_r,i,age)==1 && out.NewProjMax(old_r,i)!=-1)
          delta-=age-out.NewProjMax(old_r,i);
      }
      for(i=oad+st;i<min(nad+st,in.Days());i++){
        if(out.RoomDayMin(mv.new_r,i)>age)
          delta+=out.RoomDayMin(mv.new_r,i)-age;
        if(out.RoomDayMax(mv.new_r,i)!=-1 && out.RoomDayMax(mv.new_r,i)<age)
          delta+=age-out.RoomDayMax(mv.new_r,i);
      }
    }
    else{
      for(i=nad;i<oad;i++){
        if(out.RoomDayMin(mv.new_r,i)>age)
          delta+=out.RoomDayMin(mv.new_r,i)-age;
        if(out.RoomDayMax(mv.new_r,i)!=-1 && out.RoomDayMax(mv.new_r,i)<age)
          delta+=age-out.RoomDayMax(mv.new_r,i);
      }
      for(i=nad+st;i<min(oad+st,in.Days());i++){
        if(out.RoomDayMin(old_r,i)!=-1 && age==out.RoomDayMin(old_r,i) && out.RoomAgePres(old_r,i,age)==1 && out.NewProjMin(old_r,i)!=-1)
          delta-=out.NewProjMin(old_r,i)-age;
        else if(out.RoomDayMax(old_r,i)!=-1 && age==out.RoomDayMax(old_r,i) && out.RoomAgePres(old_r,i,age)==1 && out.NewProjMax(old_r,i)!=-1)
          delta-=age-out.NewProjMax(old_r,i);
      }
    }
  }
  return delta;
}

int IH_MoveAdmissionDeltaSkill::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,i,j;
  int old_r=out.Room(mv.patient),nad=mv.new_ad,oad=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),curn,sh,rel_sh,cursk,curn_2,rel_sh2,cursk2;
  size_t idx=0;
  vector<int>overlapping=out.FindOverlappingNurses(mv.patient,nad,mv.new_r);
  if(oad!=-1){
    for(i=oad;i<min(oad+st,in.Days());i++) 
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
          rel_sh=sh-(oad*in.ShiftsPerDay());
          curn=out.NurseAttending(old_r,sh);
          cursk=in.PatientSkillLevelRequired(mv.patient,rel_sh);
          if(in.NurseSkillLevel(curn)<cursk)
            delta-=cursk-in.NurseSkillLevel(curn);
        }
        else{ 
          rel_sh=sh-(oad*in.ShiftsPerDay());
          rel_sh2=sh-(nad*in.ShiftsPerDay());
          curn=out.NurseAttending(old_r,sh);
          curn_2=out.NurseAttending(mv.new_r,sh);
          if(curn!=curn_2)
            throw runtime_error ("Nurse "+ to_string(curn) + "!="+ to_string(curn_2)+ " but should be the same");
          cursk =in.PatientSkillLevelRequired(mv.patient,rel_sh);
          cursk2=in.PatientSkillLevelRequired(mv.patient,rel_sh2);
          if(cursk!=cursk2){ 
            if(cursk>in.NurseSkillLevel(curn) && cursk2>in.NurseSkillLevel(curn))
              delta+=cursk2-cursk;
            else if(cursk>in.NurseSkillLevel(curn) && cursk2<=in.NurseSkillLevel(curn)) 
              delta-=cursk-in.NurseSkillLevel(curn);
            else if (cursk<=in.NurseSkillLevel(curn) && cursk2>in.NurseSkillLevel(curn))
              delta+=cursk2-in.NurseSkillLevel(curn);
          }
          idx++;
        }
      }
  } 
  idx=0;
  if(nad!=-1){
    for(i=nad;i<min(nad+st,in.Days());i++) 
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
          rel_sh=sh-(nad*in.ShiftsPerDay());
          curn=out.NurseAttending(mv.new_r,sh);
          cursk=in.PatientSkillLevelRequired(mv.patient,rel_sh);
          if(in.NurseSkillLevel(curn)<cursk)
            delta+=cursk-in.NurseSkillLevel(curn);
        }
        else
          idx++;
      }
  }
  return delta;
}

int IH_MoveAdmissionDeltaExcess::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,i,j;
  int old_r=out.Room(mv.patient),nad=mv.new_ad,oad=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),curn,sh,rel_sh,curw,curn_2,rel_sh2,curw2;
  size_t idx=0;
  vector<int>overlapping=out.FindOverlappingNurses(mv.patient,nad,mv.new_r);
  if(oad!=-1){
    for(i=oad;i<min(oad+st,in.Days());i++) 
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
          rel_sh=sh-(oad*in.ShiftsPerDay());
          curn=out.NurseAttending(old_r,sh);
          curw=in.PatientWorkloadProduced(mv.patient,rel_sh);
          if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh)){ 
            if(out.NurseShiftLoad(curn,sh)-curw>in.NurseMaxLoad(curn,sh)) 
              delta-=curw;
            else  
              delta-=out.NurseShiftLoad(curn,sh)-in.NurseMaxLoad(curn,sh);
          }
        }
        else{ 
          rel_sh=sh-(oad*in.ShiftsPerDay());
          rel_sh2=sh-(nad*in.ShiftsPerDay());
          curn=out.NurseAttending(old_r,sh);
          curn_2=out.NurseAttending(mv.new_r,sh);
          if(curn!=curn_2)
            throw runtime_error ("Nurse "+ to_string(curn) + "!="+ to_string(curn_2)+ " but should be the same");
          curw=in.PatientWorkloadProduced(mv.patient,rel_sh);
          curw2=in.PatientWorkloadProduced(mv.patient,rel_sh2);
          if(curw!=curw2){ 
            if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh) && out.NurseShiftLoad(curn,sh)+(curw2-curw)>in.NurseMaxLoad(curn,sh))
              delta+=curw2-curw;
            else if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh) && out.NurseShiftLoad(curn,sh)+(curw2-curw)<=in.NurseMaxLoad(curn,sh)) 
              delta-=out.NurseShiftLoad(curn,sh)-in.NurseMaxLoad(curn,sh); 
            else if(out.NurseShiftLoad(curn,sh)<=in.NurseMaxLoad(curn,sh) && out.NurseShiftLoad(curn,sh)+(curw2-curw)>in.NurseMaxLoad(curn,sh)) 
              delta+=out.NurseShiftLoad(curn,sh)+(curw2-curw)-in.NurseMaxLoad(curn,sh);
          }
          idx++;
        }
      }
  } 
  idx=0;
  if(nad!=-1){
    for(i=nad;i<min(nad+st,in.Days());i++) 
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
          rel_sh=sh-(nad*in.ShiftsPerDay());
          curn=out.NurseAttending(mv.new_r,sh);
          curw=in.PatientWorkloadProduced(mv.patient,rel_sh);
          if(out.NurseShiftLoad(curn,sh)+curw>in.NurseMaxLoad(curn,sh)){ 
            if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh))
              delta+=curw;
            else
              delta+=out.NurseShiftLoad(curn,sh)+curw-in.NurseMaxLoad(curn,sh);
          }
        }
        else
          idx++;
      }
  }
  return delta;
}

int IH_MoveAdmissionDeltaContinuity::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0,newc=0,ad=mv.new_ad,n;
  vector<int>npr(in.Nurses(),0);
  if(mv.new_ad!=-1){
    for(int i=mv.new_ad;i<min(ad+in.PatientLengthOfStay(mv.patient),in.Days());i++){
      for(int j=0;j<in.ShiftsPerDay();j++){
        n=out.NurseAttending(mv.new_r,(i*in.ShiftsPerDay())+j);
        if(npr[n]==0)
          newc++;
        npr[n]++;
      }
    }
  }
  delta=newc-out.PatientNumNurses(mv.patient);
  return delta;
}

int IH_MoveAdmissionDeltaSurgeonTransfer::ComputeDeltaCost(const IH_Output& out,const IH_MoveAdmission& mv) const{
  int delta=0;
  int oad=out.AdmissionDay(mv.patient),o_ot=out.OT(mv.patient),s=in.PatientSurgeon(mv.patient);
  if(oad!=-1 && out.SurgeonDayTheaters(s,oad)>1 && out.SurgeonDayTheaterCount(s,oad,o_ot)==1)
    delta--;
  if(mv.new_ad!=-1 && out.SurgeonDayTheaters(s,mv.new_ad)>=1 && out.SurgeonDayTheaterCount(s,mv.new_ad,mv.new_ot)==0)
    delta++;
  return delta;
}

//Delta ChangeOT

int IH_ChangeOTDeltaOvertimeOT::ComputeDeltaCost(const IH_Output& out,const IH_ChangeOT& mv) const{
  int delta=0;
  int ad=out.AdmissionDay(mv.patient),o_ot=out.OT(mv.patient),d=in.PatientSurgeryDuration(mv.patient);
  if(out.OperatingTheaterDayLoad(o_ot,ad)>in.OperatingTheaterAvailability(o_ot,ad)){
    if(out.OperatingTheaterDayLoad(o_ot,ad)-d>in.OperatingTheaterAvailability(o_ot,ad))
      delta-=d;
    else
      delta-=out.OperatingTheaterDayLoad(o_ot,ad)-in.OperatingTheaterAvailability(o_ot,ad);
  }
  if(out.OperatingTheaterDayLoad(mv.new_ot,ad)+d>in.OperatingTheaterAvailability(mv.new_ot,ad)){
    if(out.OperatingTheaterDayLoad(mv.new_ot,ad)>in.OperatingTheaterAvailability(mv.new_ot,ad))
      delta+=d;
    else
      delta+=out.OperatingTheaterDayLoad(mv.new_ot,ad)+d-in.OperatingTheaterAvailability(mv.new_ot,ad);
  }
  return delta;
}

int IH_ChangeOTDeltaOpenOT::ComputeDeltaCost(const IH_Output& out,const IH_ChangeOT& mv) const{
  int ad=out.AdmissionDay(mv.patient);
  return out.DeltaOpenOT(mv.patient,ad,mv.new_ot);
}

int IH_ChangeOTDeltaSurgeonTransfer::ComputeDeltaCost(const IH_Output& out,const IH_ChangeOT& mv) const{
  int delta=0;
  int ad=out.AdmissionDay(mv.patient),o_ot=out.OT(mv.patient),s=in.PatientSurgeon(mv.patient);
  if(out.SurgeonDayTheaters(s,ad)>1 && out.SurgeonDayTheaterCount(s,ad,o_ot)==1 && out.SurgeonDayTheaterCount(s,ad,mv.new_ot)>0){
    delta--;
  }
  if(out.SurgeonDayTheaters(s,ad)>=1 && out.SurgeonDayTheaterCount(s,ad,mv.new_ot)==0 && out.SurgeonDayTheaterCount(s,ad,o_ot)!=1)
    delta++;
  return delta;
}

//Deltas 

int IH_ChangeRoomDeltaOverCapacity::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),d=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient);
  for(i=0;i<min(st,in.Days()-d);i++){
      if(old_r!=-1 && out.RoomDayLoad(old_r,d+i)>in.RoomCapacity(old_r))
        delta--;
      if(mv.new_r!=-1 && out.RoomDayLoad(mv.new_r,d+i)>=in.RoomCapacity(mv.new_r))
        delta++;
    }
  return delta;
}

int IH_ChangeRoomDeltaGenderMix::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),d=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient);
  for(i=0;i<min(st,in.Days()-d);i++){
    if(out.RoomDayBPatients(old_r,d+i)>0 && out.RoomDayAPatients(old_r,d+i)>0){
      if(out.RoomDayBPatients(old_r,d+i)>=out.RoomDayAPatients(old_r,d+i) && in.PatientGender(mv.patient)==Gender::A)
        delta--;
      else if(out.RoomDayBPatients(old_r,d+i)<=out.RoomDayAPatients(old_r,d+i) && in.PatientGender(mv.patient)==Gender::B)
        delta--;
    }
    if(out.RoomDayBPatients(mv.new_r,d+i)>0 || out.RoomDayAPatients(mv.new_r,d+i)>0){
      if(out.RoomDayBPatients(mv.new_r,d+i)>out.RoomDayAPatients(mv.new_r,d+i) && in.PatientGender(mv.patient)==Gender::A)
        delta++;
      else if(out.RoomDayBPatients(mv.new_r,d+i)<out.RoomDayAPatients(mv.new_r,d+i) && in.PatientGender(mv.patient)==Gender::B)
        delta++;
    }
  }
  return delta;
}

int IH_ChangeRoomDeltaAgeMix::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,i;
  int old_r=out.Room(mv.patient),d=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),age=in.PatientAgeGroup(mv.patient);
  for(i=0;i<min(st,in.Days()-d);i++){ 
    if(out.RoomDayMin(old_r,d+i)!=-1 && age==out.RoomDayMin(old_r,d+i) && out.RoomAgePres(old_r,d+i,age)==1 && out.NewProjMin(old_r,d+i)!=-1)
      delta-=out.NewProjMin(old_r,d+i)-age;
    else if(out.RoomDayMax(old_r,d+i)!=-1 && age==out.RoomDayMax(old_r,d+i) && out.RoomAgePres(old_r,d+i,age)==1 && out.NewProjMax(old_r,d+i)!=-1)
      delta-=age-out.NewProjMax(old_r,d+i);
    if(out.RoomDayMin(mv.new_r,d+i)>age)
      delta+=out.RoomDayMin(mv.new_r,d+i)-age;
    if(out.RoomDayMax(mv.new_r,d+i)!=-1 && out.RoomDayMax(mv.new_r,d+i)<age)
      delta+=age-out.RoomDayMax(mv.new_r,d+i);
  }
  return delta;
}

int IH_ChangeRoomDeltaSkill::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,i,j;
  int old_r=out.Room(mv.patient),d=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),curn,sh,rel_sh,cursk;
  size_t idx=0;
  vector<int>overlapping=out.FindOverlappingNurses(mv.patient,d,mv.new_r);
  for(i=d;i<min(d+st,in.Days());i++) 
    for(j=0;j<in.ShiftsPerDay();j++){
      sh=(i*in.ShiftsPerDay())+j;
      if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
        rel_sh=sh-(d*in.ShiftsPerDay());
        curn=out.NurseAttending(old_r,sh);
        cursk=in.PatientSkillLevelRequired(mv.patient,rel_sh);
        if(in.NurseSkillLevel(curn)<cursk)
          delta-=cursk-in.NurseSkillLevel(curn);
        curn=out.NurseAttending(mv.new_r,sh);
        cursk=in.PatientSkillLevelRequired(mv.patient,rel_sh);
        if(in.NurseSkillLevel(curn)<cursk)
          delta+=cursk-in.NurseSkillLevel(curn);
      }
      else
        idx++;
    } 
  return delta;
}

int IH_ChangeRoomDeltaExcess::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,i,j;
  int old_r=out.Room(mv.patient),d=out.AdmissionDay(mv.patient),st=in.PatientLengthOfStay(mv.patient),curn,sh,rel_sh,curw;
  size_t idx=0;
  vector<int>overlapping=out.FindOverlappingNurses(mv.patient,d,mv.new_r);
  for(i=d;i<min(d+st,in.Days());i++) 
    for(j=0;j<in.ShiftsPerDay();j++){
      sh=(i*in.ShiftsPerDay())+j;
      if(idx>=overlapping.size() || sh!=overlapping[idx]){ 
        rel_sh=sh-(d*in.ShiftsPerDay());
        curn=out.NurseAttending(old_r,sh);
        curw=in.PatientWorkloadProduced(mv.patient,rel_sh);
        if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh)){ 
          if(out.NurseShiftLoad(curn,sh)-curw>in.NurseMaxLoad(curn,sh)) 
            delta-=curw;
          else  
            delta-=out.NurseShiftLoad(curn,sh)-in.NurseMaxLoad(curn,sh);
        }
        curn=out.NurseAttending(mv.new_r,sh);
        curw=in.PatientWorkloadProduced(mv.patient,rel_sh);
          if(out.NurseShiftLoad(curn,sh)+curw>in.NurseMaxLoad(curn,sh)){ 
            if(out.NurseShiftLoad(curn,sh)>in.NurseMaxLoad(curn,sh))
              delta+=curw;
            else
              delta+=out.NurseShiftLoad(curn,sh)+curw-in.NurseMaxLoad(curn,sh);
          }
      }
      else
        idx++;
    }
  return delta;
}

int IH_ChangeRoomDeltaContinuity::ComputeDeltaCost(const IH_Output& out,const IH_ChangeRoom& mv) const{
  int delta=0,newc=0,d=out.AdmissionDay(mv.patient),n;
  vector<int>npr(in.Nurses(),0);
  for(int i=d;i<min(d+in.PatientLengthOfStay(mv.patient),in.Days());i++){
    for(int j=0;j<in.ShiftsPerDay();j++){
      n=out.NurseAttending(mv.new_r,(i*in.ShiftsPerDay())+j);
      if(npr[n]==0)
        newc++;
      npr[n]++;
    }
  }
  delta=newc-out.PatientNumNurses(mv.patient);
  return delta;
}

//Change Nurse

int IH_ChangeNurseDeltaSkill::ComputeDeltaCost(const IH_Output& out,const IH_ChangeNurse& mv) const{
  int delta=0,d=mv.sh/in.ShiftsPerDay(),p,rel_sh,i,sk;
  int r=mv.r,on=out.NurseAttending(r,mv.sh);
  if(in.NurseSkillLevel(on)!=in.NurseSkillLevel(mv.nn)){
    for(i=0;i<out.RoomDayLoad(mv.r,d);i++){
      p=out.RoomDayPatient(mv.r,d,i);
      if(p<in.Patients()){
        rel_sh=mv.sh-(out.AdmissionDay(p)*in.ShiftsPerDay());
        sk=in.PatientSkillLevelRequired(p,rel_sh);
      }
      else
        sk=in.OccupantSkillLevelRequired(p-in.Patients(),mv.sh);
      if(sk>0){
        if(sk>in.NurseSkillLevel(on)){
          if(sk<=in.NurseSkillLevel(mv.nn))
            delta-=sk-in.NurseSkillLevel(on);
          else
            delta+=in.NurseSkillLevel(on)-in.NurseSkillLevel(mv.nn);
        } 
        else if(sk>in.NurseSkillLevel(mv.nn))
          delta+=sk-in.NurseSkillLevel(mv.nn);
      }
    }
  }
  return delta;
}

int IH_ChangeNurseDeltaExcess::ComputeDeltaCost(const IH_Output& out,const IH_ChangeNurse& mv) const{
  int delta=0,on=out.NurseAttending(mv.r,mv.sh),d=mv.sh/in.ShiftsPerDay(),lp=0,i,p,rel_sh;
  for(i=0;i<out.RoomDayLoad(mv.r,d);i++){
    p=out.RoomDayPatient(mv.r,d,i);
    if(p<in.Patients()){
      rel_sh=mv.sh-(out.AdmissionDay(p)*in.ShiftsPerDay());
      lp+=in.PatientWorkloadProduced(p,rel_sh);
    }
    else
      lp+=in.OccupantWorkloadProduced(p-in.Patients(),mv.sh);
  }
  if(out.NurseShiftLoad(on,mv.sh)>in.NurseMaxLoad(on,mv.sh)){
    if(out.NurseShiftLoad(on,mv.sh)-lp<in.NurseMaxLoad(on,mv.sh))
      delta-=out.NurseShiftLoad(on,mv.sh)-in.NurseMaxLoad(on,mv.sh);
    else
      delta-=lp;
  }
  if(out.NurseShiftLoad(mv.nn,mv.sh)+lp>in.NurseMaxLoad(mv.nn,mv.sh)){
    if(out.NurseShiftLoad(mv.nn,mv.sh)<in.NurseMaxLoad(mv.nn,mv.sh))
      delta+=out.NurseShiftLoad(mv.nn,mv.sh)+lp-in.NurseMaxLoad(mv.nn,mv.sh);
    else
      delta+=lp;
  }
  return delta;
}

int IH_ChangeNurseDeltaContinuity::ComputeDeltaCost(const IH_Output& out,const IH_ChangeNurse& mv) const
{
  int delta=0,on=out.NurseAttending(mv.r,mv.sh),i,p,d=mv.sh/in.ShiftsPerDay();
  for(i=0;i<out.RoomDayLoad(mv.r,d);i++){
    p=out.RoomDayPatient(mv.r,d,i);
      if(out.Assigned(p,on)==1 && out.Assigned(p,mv.nn)>0)
        delta--;
      else if(out.Assigned(p,mv.nn)==0 && out.Assigned(p,on)>1)
        delta++; 
  }
  return delta;
}

//Delta SwapPatients

int IH_SwapPatientsDeltaOvertimeSurgeon::ComputeDeltaCost(const IH_Output& out,const IH_SwapPatients& mv) const
{
  int delta=0;
  if(mv.day1==-1 || mv.day2==-1)
    return out.InOutDeltaOvertimeSurgeon(mv.p1,mv.p2);
  if(in.PatientSurgeon(mv.p1)!=in.PatientSurgeon(mv.p2)){ 
    if(mv.day1!=mv.day2){
      delta+=out.DeltaSurgeonOvertimeMP(mv.p1,mv.day2);
      delta+=out.DeltaSurgeonOvertimeMP(mv.p2,mv.day1);
    }
  }
  else{ 
    int difference=in.PatientSurgeryDuration(mv.p1)-in.PatientSurgeryDuration(mv.p2),s=in.PatientSurgeon(mv.p1);
    if(difference!=0 && mv.day1!=mv.day2){ 
      if(mv.day1!=-1){
        if(out.SurgeonDayLoad(s,mv.day1)>in.SurgeonMaxSurgeryTime(s,mv.day1)){
          if(out.SurgeonDayLoad(s,mv.day1)-difference>in.SurgeonMaxSurgeryTime(s,mv.day1))
            delta-=difference;
          else
            delta-=out.SurgeonDayLoad(s,mv.day1)-in.SurgeonMaxSurgeryTime(s,mv.day1);
        }
        else{
          if(out.SurgeonDayLoad(s,mv.day1)-difference>in.SurgeonMaxSurgeryTime(s,mv.day1))
            delta-=in.SurgeonMaxSurgeryTime(s,mv.day1)-(out.SurgeonDayLoad(s,mv.day1)-difference);
        }
      }
      if(mv.day2!=-1){
        if(out.SurgeonDayLoad(s,mv.day2)>in.SurgeonMaxSurgeryTime(s,mv.day2)){
          if(out.SurgeonDayLoad(s,mv.day2)+difference>in.SurgeonMaxSurgeryTime(s,mv.day2))
            delta+=difference;
          else
            delta+=in.SurgeonMaxSurgeryTime(s,mv.day2)-out.SurgeonDayLoad(s,mv.day2);
        }
        else{
          if(out.SurgeonDayLoad(s,mv.day2)+difference>in.SurgeonMaxSurgeryTime(s,mv.day2))
            delta+=out.SurgeonDayLoad(s,mv.day2)+difference-in.SurgeonMaxSurgeryTime(s,mv.day2);
        }
      }
    }  
  }
  return delta;
}

int IH_SwapPatientsDeltaOvertimeOT::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0,difference=in.PatientSurgeryDuration(mv.p1)-in.PatientSurgeryDuration(mv.p2),ot1=out.OT(mv.p1),ot2=out.OT(mv.p2);
  if(mv.day1==-1 || mv.day2==-1)
    out.InOutDeltaOvertimeOT(mv.p1,mv.p2);
  if(mv.day1!=mv.day2 || ot1!=ot2){
    if(mv.day1!=-1){
      if(out.OperatingTheaterDayLoad(ot1,mv.day1)>in.OperatingTheaterAvailability(ot1,mv.day1)){
        if(out.OperatingTheaterDayLoad(ot1,mv.day1)-difference>in.OperatingTheaterAvailability(ot1,mv.day1)) 
          delta-=difference;
        else
          delta-=out.OperatingTheaterDayLoad(ot1,mv.day1)-in.OperatingTheaterAvailability(ot1,mv.day1); 
      }
      else{
        if(out.OperatingTheaterDayLoad(ot1,mv.day1)-difference>in.OperatingTheaterAvailability(ot1,mv.day1))
          delta-=in.OperatingTheaterAvailability(ot1,mv.day1)-(out.OperatingTheaterDayLoad(ot1,mv.day1)-difference);
      }
    }
    if(mv.day2!=-1){
      if(out.OperatingTheaterDayLoad(ot2,mv.day2)>in.OperatingTheaterAvailability(ot2,mv.day2)){
        if(out.OperatingTheaterDayLoad(ot2,mv.day2)+difference>in.OperatingTheaterAvailability(ot2,mv.day2)) 
          delta+=difference;
        else
          delta+=in.OperatingTheaterAvailability(ot2,mv.day2)-out.OperatingTheaterDayLoad(ot2,mv.day2); 
      }
      else{
        if(out.OperatingTheaterDayLoad(ot2,mv.day2)+difference>in.OperatingTheaterAvailability(ot2,mv.day2))
          delta+=out.OperatingTheaterDayLoad(ot2,mv.day2)+difference-in.OperatingTheaterAvailability(ot2,mv.day2);
      }
    }
  }
  return delta;
}

int IH_SwapPatientsDeltaOverCapacity::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0;
  int r1=out.Room(mv.p1),r2=out.Room(mv.p2),st1=in.PatientLengthOfStay(mv.p1),st2=in.PatientLengthOfStay(mv.p2);
  int ed1=mv.day1==-1?-1:min(mv.day1+st1,in.Days()),ed2=mv.day2==-1?-1:min(mv.day2+st2,in.Days());
  int fed2=mv.day1==-1?-1:min(mv.day1+st2,in.Days()),fed1=mv.day2==-1?-1:min(mv.day2+st1,in.Days());
  bool olp=((mv.day1<mv.day2 && (mv.day2<ed1)) || (mv.day2<mv.day1 && (mv.day1<ed2)));
  bool folp=((mv.day1<mv.day2 && (mv.day2<fed2)) || (mv.day2<mv.day1 && (mv.day1<fed1)));
  if(st1!=st2){
    if(r1!=r2 || (!olp && !folp)) 
      delta=out.NotOverlappingSwapOvercapacity(mv.p1,mv.p2);
    else
      delta=out.OverlappingSwapOvercapacity(mv.p1,mv.p2);
  }
  return delta;
}

int IH_SwapPatientsDeltaDelay::ComputeDeltaCost(const IH_Output&, const IH_SwapPatients& mv) const{
  int delta=0, prev_delay, new_delay;
  if(mv.day1!=mv.day2){
    if(mv.day1!=-1){
      prev_delay=(mv.day2!=-1)? mv.day2-in.PatientSurgeryReleaseDay(mv.p2):0;
      new_delay=mv.day1-in.PatientSurgeryReleaseDay(mv.p2);
      delta-=prev_delay-new_delay;
    }
    else 
      delta-=mv.day2-in.PatientSurgeryReleaseDay(mv.p2);
    if(mv.day2!=-1){
      prev_delay=(mv.day1!=-1)? mv.day1-in.PatientSurgeryReleaseDay(mv.p1):0;
      new_delay=mv.day2-in.PatientSurgeryReleaseDay(mv.p1);
      delta-=prev_delay-new_delay;
    }
    else
      delta-=mv.day1-in.PatientSurgeryReleaseDay(mv.p1);
  }
  return delta;
}

int IH_SwapPatientsDeltaGenderMix::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0;
  int r1=out.Room(mv.p1),r2=out.Room(mv.p2),st1=in.PatientLengthOfStay(mv.p1),st2=in.PatientLengthOfStay(mv.p2);
  int ed1=mv.day1==-1?-1:min(mv.day1+st1,in.Days()),ed2=mv.day2==-1?-1:min(mv.day2+st2,in.Days());
  int fed2=mv.day1==-1?-1:min(mv.day1+st2,in.Days()),fed1=mv.day2==-1?-1:min(mv.day2+st1,in.Days());
  bool olp=((mv.day1<mv.day2 && (mv.day2<ed1)) || (mv.day2<mv.day1 && (mv.day1<ed2)));
  bool folp=((mv.day1<mv.day2 && (mv.day2<fed2)) || (mv.day2<mv.day1 && (mv.day1<fed1)));
  if(r1!=r2 || (!olp && !folp)) 
    delta+=out.NotOverlappingSwapGenderMix(mv.p1,mv.p2);
  else
    delta+=out.OverlappingSwapGenderMix(mv.p1,mv.p2);
  return delta;
}

int IH_SwapPatientsDeltaAgeMix::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0;
  int r1=out.Room(mv.p1),r2=out.Room(mv.p2),st1=in.PatientLengthOfStay(mv.p1),st2=in.PatientLengthOfStay(mv.p2);
  int ed1=mv.day1==-1?-1:min(mv.day1+st1,in.Days()),ed2=mv.day2==-1?-1:min(mv.day2+st2,in.Days());
  int fed2=mv.day1==-1?-1:min(mv.day1+st2,in.Days()),fed1=mv.day2==-1?-1:min(mv.day2+st1,in.Days());
  bool olp=((mv.day1<mv.day2 && (mv.day2<ed1)) || (mv.day2<mv.day1 && (mv.day1<ed2)));
  bool folp=((mv.day1<mv.day2 && (mv.day2<fed2)) || (mv.day2<mv.day1 && (mv.day1<fed1)));
  if(r1!=r2 || (!olp && !folp))
    delta+=out.NotOverlappingSwapAgeMix(mv.p1,mv.p2);
  else
    delta+=out.OverlappingSwapAgeMix(mv.p1,mv.p2);
  return delta;
}

int IH_SwapPatientsDeltaSkill::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  return out.SwapSkill(mv.p1,mv.p2);
}

int IH_SwapPatientsDeltaExcess::ComputeDeltaCost(const IH_Output& out,const IH_SwapPatients& mv) const{
  int delta=0,n,i,j,diff;
  int r1=out.Room(mv.p1),r2=out.Room(mv.p2),st1=in.PatientLengthOfStay(mv.p1),st2=in.PatientLengthOfStay(mv.p2);
  int ed1=mv.day1==-1?-1:min(mv.day1+st1,in.Days()),ed2=mv.day2==-1?-1:min(mv.day2+st2,in.Days());
  int fed2=mv.day1==-1?-1:min(mv.day1+st2,in.Days()),fed1=mv.day2==-1?-1:min(mv.day2+st1,in.Days());
  bool olp=((mv.day1<=mv.day2 && (mv.day2<ed1)) || (mv.day2<=mv.day1 && (mv.day1<ed2)));
  bool folp=((mv.day1<=mv.day2 && (mv.day2<fed2)) || (mv.day2<=mv.day1 && (mv.day1<fed1)));
  int sh,rel_sh,wl1,wl2;
  if(mv.day1==mv.day2 && r1==r2)
    return delta;
  if(olp || folp)
    return out.OverlappingSwapExcess(mv.p1,mv.p2);
  if(st1>st2){
    for(i=mv.day1;i<ed1;i++)
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(mv.day1*in.ShiftsPerDay());
        n=out.NurseAttending(r1,(i*in.ShiftsPerDay())+j);
        wl1=in.PatientWorkloadProduced(mv.p1,rel_sh);
        if(i<fed2)
          wl2=in.PatientWorkloadProduced(mv.p2,rel_sh);
        else
          wl2=0;
        diff=wl2-wl1;
        delta+=out.WorkloadDelta(n,sh,diff);
      } 
  for(i=mv.day2;i<fed1;i++) 
    for(j=0;j<in.ShiftsPerDay();j++){
      sh=(i*in.ShiftsPerDay())+j;
      rel_sh=sh-(mv.day2*in.ShiftsPerDay());
      n=out.NurseAttending(r2,(i*in.ShiftsPerDay())+j);
      wl1=in.PatientWorkloadProduced(mv.p1,rel_sh);
      if(i<ed2)
        wl2=in.PatientWorkloadProduced(mv.p2,rel_sh);
      else
        wl2=0;
      diff=wl1-wl2;
      delta+=out.WorkloadDelta(n,sh,diff);
    }
  }
  else{
    for(i=mv.day1;i<fed2;i++)
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(mv.day1*in.ShiftsPerDay());
        n=out.NurseAttending(r1,(i*in.ShiftsPerDay())+j);
        wl2=in.PatientWorkloadProduced(mv.p2,rel_sh);
        if(i<ed1)
          wl1=in.PatientWorkloadProduced(mv.p1,rel_sh);
        else
          wl1=0;
        diff=wl2-wl1;
        delta+=out.WorkloadDelta(n,sh,diff);
      }
    for(i=mv.day2;i<ed2;i++)
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(mv.day2*in.ShiftsPerDay());
        n=out.NurseAttending(r2,(i*in.ShiftsPerDay())+j);
        wl2=in.PatientWorkloadProduced(mv.p2,rel_sh);
        if(i<fed1)
          wl1=in.PatientWorkloadProduced(mv.p1,rel_sh);
        else
          wl1=0;
        diff=wl1-wl2;
        delta+=out.WorkloadDelta(n,sh,diff);
      }
  }
  return delta;
}

int IH_SwapPatientsDeltaContinuity::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0,n;
  int r1=out.Room(mv.p1),r2=out.Room(mv.p2),st1=in.PatientLengthOfStay(mv.p1),st2=in.PatientLengthOfStay(mv.p2);
  int ed1=mv.day1==-1?-1:min(mv.day1+st1,in.Days()),ed2=mv.day2==-1?-1:min(mv.day2+st2,in.Days());
  int fed2=mv.day1==-1?-1:min(mv.day1+st2,in.Days()),fed1=mv.day2==-1?-1:min(mv.day2+st1,in.Days());
  vector<bool> found(in.Nurses(),false);
  vector<int> count(in.Nurses(),0);
  if(st1>st2){ 
    for(int i=fed2;i<ed1;i++)
      for(int j=0;j<in.ShiftsPerDay();j++){
        n=out.NurseAttending(r1,(i*in.ShiftsPerDay())+j);
        count[n]++;
        if(count[n]==out.Assigned(mv.p1,n))
          delta--;
      }
    for(int i=ed2;i<fed1;i++)
      for(int j=0;j<in.ShiftsPerDay();j++){
        n=out.NurseAttending(r2,(i*in.ShiftsPerDay())+j);
        if(out.Assigned(mv.p2,n)==0 && !found[n]){
          delta++;
          found[n]=true;
        }
      }
  } 
  else{
    for(int i=ed1;i<fed2;i++)
      for(int j=0;j<in.ShiftsPerDay();j++){
        n=out.NurseAttending(r1,(i*in.ShiftsPerDay())+j);
        if(out.Assigned(mv.p1,n)==0 && !found[n]){
          delta++;
          found[n]=true;
        }
      }
    for(int i=fed1;i<ed2;i++)
      for(int j=0;j<in.ShiftsPerDay();j++){
        n=out.NurseAttending(r2,(i*in.ShiftsPerDay())+j);
        count[n]++;
        if(count[n]==out.Assigned(mv.p2,n))
          delta--;
        }
  }
  return delta;
  }

int IH_SwapPatientsDeltaSurgeonTransfer::ComputeDeltaCost(const IH_Output& out, const IH_SwapPatients& mv) const{
  int delta=0;
  int as1=in.PatientSurgeon(mv.p1),as2=in.PatientSurgeon(mv.p2);
  if(as1!=as2 && mv.day1!=mv.day2){
    if(mv.day1!=-1){
      if(out.SurgeonDayTheaters(as1,mv.day1)>1 && out.SurgeonDayTheaterCount(as1,mv.day1,out.OT(mv.p1))==1)
        delta--;
      if(out.SurgeonDayTheaters(as2,mv.day1)>=1 && out.SurgeonDayTheaterCount(as2,mv.day1,out.OT(mv.p1))==0)
        delta++;
    }
    if(mv.day2!=-1){
      if(out.SurgeonDayTheaters(as2,mv.day2)>1 && out.SurgeonDayTheaterCount(as2,mv.day2,out.OT(mv.p2))==1)
        delta--;
      if(out.SurgeonDayTheaters(as1,mv.day2)>=1 && out.SurgeonDayTheaterCount(as1,mv.day2,out.OT(mv.p2))==0)
        delta++;
    }
  }
  if(as1!=as2 && mv.day1==mv.day2 && out.OT(mv.p1)!=out.OT(mv.p2)){ 
    if(out.SurgeonDayTheaters(as1,mv.day1)>1 && out.SurgeonDayTheaterCount(as1,mv.day1,out.OT(mv.p1))==1 && out.SurgeonDayTheaterCount(as1,mv.day1,out.OT(mv.p2))>0)
      delta--;
    if(out.SurgeonDayTheaters(as2,mv.day1)>1 && out.SurgeonDayTheaterCount(as2,mv.day1,out.OT(mv.p2))==1 && out.SurgeonDayTheaterCount(as2,mv.day1,out.OT(mv.p1))>0)
      delta--;
    if(out.SurgeonDayTheaters(as1,mv.day1)>=1 && out.SurgeonDayTheaterCount(as1,mv.day1,out.OT(mv.p2))==0 && out.SurgeonDayTheaterCount(as1,mv.day1,out.OT(mv.p1))>1)
      delta++;
    if(out.SurgeonDayTheaters(as2,mv.day1)>=1 && out.SurgeonDayTheaterCount(as2,mv.day1,out.OT(mv.p1))==0 && out.SurgeonDayTheaterCount(as2,mv.day1,out.OT(mv.p2))>1)
      delta++;
  }
  return delta;
}
