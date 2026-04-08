// File IH_Output.cc
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "json.hpp"
#include "IH_Output.hh"

IH_Output::IH_Output(const IH_Input& my_in)
  : in(my_in), admission_day(in.Patients(),-1), room(in.Patients(),-1), 
   operating_room(in.Patients(),-1),
   scheduled(0),
   patient_num_nurses(in.Patients()+in.Occupants(),0),
   ever_assigned(in.Patients()+in.Occupants(),vector<int>(in.Nurses(),0)),
   room_day_patient_list(in.Rooms(), vector<vector<int>>(in.Days())),
   room_day_b_patients(in.Rooms(), vector<int>(in.Days(),0)),
   room_day_a_patients(in.Rooms(), vector<int>(in.Days(),0)),
   room_shift_nurse(in.Rooms(),vector<int>(in.Shifts(),-1)),
   nurse_shift_room_list(in.Nurses(),vector<vector<int>>(in.Shifts())),
   nurse_shift_load(in.Nurses(),vector<int>(in.Shifts(),0)),
   room_day_age_distribution(in.Rooms(),vector<vector<int>>(in.Days(),vector<int>(in.AgeGroups(),0))),
   min_max(in.Rooms(),vector<pair<int,int>>(in.Days(),make_pair(-1,-1))),
   operatingtheater_day_patient_list(in.OperatingTheaters(),vector<vector<int>>(in.Days())),
   operatingtheater_day_load(in.OperatingTheaters(), vector<int>(in.Days(),0)),
   surgeon_day_load(in.Surgeons(), vector<int>(in.Days(),0)),
   surgeon_day_theater_count(in.Surgeons(), vector<vector<int>>(in.Days(), vector<int>(in.OperatingTheaters(),0))),
   surgeon_day_theaters_visited(in.Surgeons(), vector<int>(in.Days(),0)),
   daily_admissions(in.Days(),vector<int>())
{}

IH_Output& IH_Output::operator=(const IH_Output& out)
{
  admission_day = out.admission_day;
  room = out.room;
  operating_room = out.operating_room;
  scheduled=out.scheduled;
  ever_assigned=out.ever_assigned;
  patient_num_nurses=out.patient_num_nurses;
  room_day_patient_list = out.room_day_patient_list; 
  room_day_age_distribution = out.room_day_age_distribution;
  min_max=out.min_max;
  room_day_b_patients = out.room_day_b_patients;
  room_day_a_patients = out.room_day_a_patients;
  room_shift_nurse = out.room_shift_nurse;
  nurse_shift_room_list = out.nurse_shift_room_list;
  nurse_shift_load = out.nurse_shift_load;
  operatingtheater_day_patient_list = out.operatingtheater_day_patient_list;
  operatingtheater_day_load = out.operatingtheater_day_load;
  surgeon_day_load = out.surgeon_day_load;
  surgeon_day_theater_count = out.surgeon_day_theater_count;
  surgeon_day_theaters_visited = out.surgeon_day_theaters_visited;
  daily_admissions=out.daily_admissions;
  return *this;
}

ostream& operator<<(ostream& os, const IH_Output& out)
{
  int p, r, s, t, d, i;
  os << "Patients<id,admission_day,room,op_room>: ";
  for (p = 0; p < out.in.Patients(); p++)
    os << "<" << out.in.PatientId(p) << "," 
       << out.admission_day[p] << ","
       << out.room[p] << ","
       << out.operating_room[p]<< "> ";
  os << endl;
  os << endl;

  os << "Coverage: " << endl;
  for (r = 0; r < out.in.Rooms(); r++)
  {
    os << out.in.RoomId(r) << " ";
    for (s = 0; s < out.in.Shifts(); s++)
    { 
      if (s % out.in.ShiftsPerDay() == 0)
        os << "| ";
      if(out.room_day_patient_list[r][s/out.in.ShiftsPerDay()].size()==0)
        os<<"---"<< " ";
      else
        os << out.in.NurseId(out.room_shift_nurse[r][s]) << " ";
    }
    os << "| " << endl;
  }
  os << endl;

  os << "Rooms: " << endl;
  string pat;
  for (r = 0; r < out.in.Rooms(); r++)
  {
    os << out.in.RoomId(r) <<" ["<<out.in.RoomCapacity(r)<<"] | ";	
       for (d = 0; d < out.in.Days(); d++)
       { 
        string gender;         
        if (out.room_day_patient_list[r][d].size() == 0 && out.in.OccupantsPresent(r,d)==0)
          os << "--- ";
        for (i = 0; i < out.in.OccupantsPresent(r,d); i++){
          gender=(out.in.OccupantGender(out.in.OccupantPresence(r,d,i))==Gender::A)?"A) ":"B) ";
          os << out.in.OccupantId(out.in.OccupantPresence(r,d,i)) <<"("<<gender;
        }        
        for (i = 0; i < out.room_day_patient_list[r][d].size(); i++){
          if(out.room_day_patient_list[r][d][i]<out.in.Patients()){
            gender=(out.in.PatientGender(out.room_day_patient_list[r][d][i])==Gender::A)?"A) ":"B) ";
            os << out.in.PatientId(out.room_day_patient_list[r][d][i]) <<"("<<gender;
          }
        }
        os << "| ";   
       }
       os << endl;
  }
    os << "Operating theaters: " << endl;
    for (t = 0; t < out.in.OperatingTheaters(); t++)
    {
      os << out.in.OperatingTheaterId(t) << " | ";
       for (d = 0; d < out.in.Days(); d++)
       {          
          if(out.in.OperatingTheaterAvailability(t,d)==0)
            os << "XXX ";
          else if (out.operatingtheater_day_patient_list[t][d].size() == 0)
            os << "--- ";          
          for (i = 0; i < out.operatingtheater_day_patient_list[t][d].size(); i++)
             os << out.in.PatientId(out.operatingtheater_day_patient_list[t][d][i]) << " ";
           os << "| ";   
       }
       os << endl;
    }
    return os;
}

istream& operator>>(istream& is, IH_Output& out)
{
	nlohmann::json j_sol,j_n,j_p;
    is >> j_sol;
	int p,d,r,t,n,i,j,s,ind,scheduled=0;
  string nurse_id,patient_id,shift_name,room_id;
	out.Reset();
    for (i = 0; i < j_sol["patients"].size(); i++)
	  {
	    j_p = j_sol["patients"][i];
	    if (j_p["admission_day"] != "none")
	    {
	       patient_id = j_p["id"];
	  	 p = out.in.FindPatient(patient_id);
	  	 d = j_p["admission_day"];
	  	 r = out.in.FindRoom(j_p["room"]);
	  	 t = out.in.FindOT(j_p["operating_theater"]);
	  	 out.AssignPatient(p,d,r,t);
       if(!out.in.PatientMandatory(p))
        scheduled++;
	    }		  
	  }
    out.SetScheduled(scheduled);
    out.UpdatewithOccupantsInfo();
	  for (n = 0; n < j_sol["nurses"].size(); n++)
	  {
	    j_n = j_sol["nurses"][n];
	    nurse_id = j_n["id"];	
      ind=out.in.FindNurse(nurse_id);
	    for (i = 0; i < j_n["assignments"].size(); i++)
	    {
	  	  d = j_n["assignments"][i]["day"];
	  	  shift_name = j_n["assignments"][i]["shift"];
	  	  s = d * out.in.ShiftsPerDay() + out.in.FindShift(shift_name);
	  	  for (j = 0; j < j_n["assignments"][i]["rooms"].size(); j++)
	  	  {
	  		  room_id = j_n["assignments"][i]["rooms"][j];
	  		  r = out.in.FindRoom(room_id);
	  		  out.AssignNurse(ind,r,s);
	  	  }
	    }
	  }
	return is;
}


bool operator==(const IH_Output& out1, const IH_Output& out2)
{
	return out1.admission_day == out2.admission_day &&
         out1.room == out2.room && out1.operating_room == out2.operating_room &&
         out1.room_shift_nurse == out2.room_shift_nurse;
}

void IH_Output::Reset()
{
    int p, r, s, t, d, n;
    scheduled=0;
    for (p = 0; p < in.Patients(); p++)
    {
      admission_day[p] = -1;
      room[p] = -1;
      operating_room[p] = -1;
      patient_num_nurses[p]=0;
      for(n=0;n<in.Nurses();n++)
        ever_assigned[p][n]=0;
    }
    for (r = 0; r < in.Rooms(); r++){
      for (s = 0; s < in.Shifts(); s++)
          room_shift_nurse[r][s] = -1;
    }
    for (n = 0; n < in.Nurses(); n++){
      for (s = 0; s < in.Shifts(); s++)
      {
          nurse_shift_room_list[n][s].clear();
          nurse_shift_load[n][s] = 0;
      }
    }
    for (r = 0; r < in.Rooms(); r++){
      for (d = 0; d < in.Days(); d++)
      {
          room_day_patient_list[r][d].clear();
          room_day_b_patients[r][d] = 0;
          room_day_a_patients[r][d] = 0;
          min_max[r][d]=make_pair(-1,-1);
          for(s=0;s<in.AgeGroups();s++)
           room_day_age_distribution[r][d][s]=0;
      }
    }
    for (t = 0; t < in.OperatingTheaters(); t++){
      for (d = 0; d < in.Days(); d++)
      {
        operatingtheater_day_patient_list[t][d].clear();
        operatingtheater_day_load[t][d] = 0;
      }
    }
    for (s = 0; s < in.Surgeons(); s++){
      for (d = 0; d < in.Days(); d++)
      {
        surgeon_day_load[s][d] = 0;
        surgeon_day_theaters_visited[s][d]= 0;
        for (t = 0; t < in.OperatingTheaters(); t++)
          surgeon_day_theater_count[s][d][t] = 0;
      }
    }
    for(d = 0; d < in.Days(); d++)
      daily_admissions[d].clear();
 }

void IH_Output::PrettyPrint(ostream& os,string info) const{  
  int p, n, r, s, i;
  nlohmann::ordered_json j_out, j_p, j_n, j_s, j_r, j_t, j_u;
  
  for (p = 0; p < in.Patients(); p++)
  {
    j_p["id"] = in.PatientId(p);
    if (admission_day[p] != -1)
    {
      j_p["admission_day"] = admission_day[p];
      j_p["room"] = in.RoomId(room[p]);
      j_p["operating_theater"] = in.OperatingTheaterId(operating_room[p]);
    }
    else
      j_p["admission_day"] = "none";
    j_out["patients"].emplace_back(j_p);
    j_p.clear();
  } 
  
  for (n = 0; n < in.Nurses(); n++)
  {
     j_n["id"] = in.NurseId(n);
     for (i = 0; i < in.NurseWorkingShifts(n); i++)
     {
      s = in.NurseWorkingShift(n,i);
	    j_s["day"] = s/in.ShiftsPerDay();
	    j_s["shift"] = in.ShiftName(s % in.ShiftsPerDay());
      j_s["rooms"] = nlohmann::json::array();
      for (r = 0; r < in.Rooms(); r++){
         if (room_shift_nurse[r][s] == n)
            j_s["rooms"].emplace_back(in.RoomId(r));
      }
      j_n["assignments"].emplace_back(j_s);     
      j_s.clear();     
     }
     j_out["nurses"].emplace_back(j_n);
     j_n.clear();
  }
  j_out["costs"].emplace_back(info);
  os << j_out.dump(2);
}

void IH_Output::UpdatewithOccupantsInfo(){
  int offset=in.Patients(),i,o,d,r;
  for(i=0;i<in.Occupants();i++){
    o=i+offset;
    r=in.OccupantRoom(i);
    for(d=0;d<in.OccupantLengthOfStay(i);d++){
      room_day_patient_list[r][d].emplace_back(o);
      room_day_age_distribution[r][d][in.OccupantAgeGroup(i)]++;
      if(min_max[r][d].first==-1 && min_max[r][d].second==-1){ 
        min_max[r][d].first=in.OccupantAgeGroup(i);
        min_max[r][d].second=in.OccupantAgeGroup(i);
      }
      if(min_max[r][d].first>in.OccupantAgeGroup(i))
        min_max[r][d].first=in.OccupantAgeGroup(i);
      if(min_max[r][d].second<in.OccupantAgeGroup(i))
        min_max[r][d].second=in.OccupantAgeGroup(i);
      if (in.OccupantGender(i) == Gender::A)
        room_day_a_patients[r][d]++;
      else
       room_day_b_patients[r][d]++;
    }
  }
}

void IH_Output::AssignPatient(int p, int ad, int r, int t)
{
  int d, u;
  admission_day[p] = ad;
  room[p] = r;
  operating_room[p] = t;
  for (d = ad; d < min(in.Days(),ad + in.PatientLengthOfStay(p)); d++)
  {
    room_day_patient_list[r][d].emplace_back(p);
    room_day_age_distribution[r][d][in.PatientAgeGroup(p)]++;
    if(min_max[r][d].first==-1 && min_max[r][d].second==-1){ 
      min_max[r][d].first=in.PatientAgeGroup(p);
      min_max[r][d].second=in.PatientAgeGroup(p);
    }
    if(min_max[r][d].first>in.PatientAgeGroup(p))
      min_max[r][d].first=in.PatientAgeGroup(p);
    if(min_max[r][d].second<in.PatientAgeGroup(p))
      min_max[r][d].second=in.PatientAgeGroup(p);
    if (in.PatientGender(p) == Gender::A)
       room_day_a_patients[r][d]++;
    else
       room_day_b_patients[r][d]++;
  }
  operatingtheater_day_patient_list[t][ad].emplace_back(p);
  operatingtheater_day_load[t][ad] += in.PatientSurgeryDuration(p);
  daily_admissions[ad].emplace_back(p);
  u = in.PatientSurgeon(p);
  surgeon_day_load[u][ad] += in.PatientSurgeryDuration(p);
  surgeon_day_theater_count[u][ad][t]++;
  if(surgeon_day_theater_count[u][ad][t]==1)
    surgeon_day_theaters_visited[u][ad]++;
}

void IH_Output::AssignNurse(int n, int r, int s)
{
  int d, i, p, s1;
  if (!in.IsNurseWorkingInShift(n,s))
  {
    stringstream ss;
    ss << "Assigning a non-working nurse " << n << " to shift " << in.ShiftDescription(s);
    throw invalid_argument(ss.str());
  }
  room_shift_nurse[r][s] = n;
  nurse_shift_room_list[n][s].emplace_back(r);
  d = s/in.ShiftsPerDay();
  for (i = 0; i < room_day_patient_list[r][d].size(); i++)
  { 
	  p = room_day_patient_list[r][d][i];
    if(p<in.Patients()){
      s1 = s - admission_day[p] * in.ShiftsPerDay();
      nurse_shift_load[n][s] += in.PatientWorkloadProduced(p,s1);
    }
    else{ 
      s1=s;
      nurse_shift_load[n][s] += in.OccupantWorkloadProduced(p-in.Patients(),s);
    }
    if(ever_assigned[p][n]==0)
      patient_num_nurses[p]++;
    ever_assigned[p][n]++;
  }
}

void IH_Output::MoveAdmission(int p,int nad,int nar,int naot){
  int i,j,sh,as,an,oar,oaot;
  int oad=admission_day[p];
  as=in.PatientSurgeon(p);
  admission_day[p]=nad;
  oar=room[p];
  oaot=operating_room[p];
  room[p]=nar;
  operating_room[p]=naot;
  if(oad==-1)
    scheduled++;
  else{ 
    surgeon_day_load[as][oad]-=in.PatientSurgeryDuration(p);
    operatingtheater_day_load[oaot][oad]-=in.PatientSurgeryDuration(p);
    surgeon_day_theater_count[as][oad][oaot]--;
    if(surgeon_day_theater_count[as][oad][oaot]==0)
      surgeon_day_theaters_visited[as][oad]--;
    i=FindPatientinOT(p,oaot,oad);
    daily_admissions[oad].erase(daily_admissions[oad].begin()+FindPatientinDailyAdmission(p,oad));
    operatingtheater_day_patient_list[oaot][oad].erase(operatingtheater_day_patient_list[oaot][oad].begin()+i);
    int rel_sh;
    for(i=oad;i<min(oad+in.PatientLengthOfStay(p),in.Days());i++){
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        an=NurseAttending(oar,sh);
        rel_sh=sh-(oad*in.ShiftsPerDay());
        nurse_shift_load[an][sh]-=in.PatientWorkloadProduced(p,rel_sh);
        ever_assigned[p][an]--;
        if(ever_assigned[p][an]==0)
          patient_num_nurses[p]--;
      }
      if(in.PatientGender(p)==Gender::A)
        room_day_a_patients[oar][i]--;
      else
        room_day_b_patients[oar][i]--;
      j=FindPatientinRoom(p,oar,i);
      room_day_patient_list[oar][i].erase(room_day_patient_list[oar][i].begin()+j);
      room_day_age_distribution[oar][i][in.PatientAgeGroup(p)]--;
      if(min_max[oar][i].first==in.PatientAgeGroup(p) && room_day_age_distribution[oar][i][in.PatientAgeGroup(p)]==0)
        min_max[oar][i].first=NewMin(oar,i);
      if(min_max[oar][i].second==in.PatientAgeGroup(p) && room_day_age_distribution[oar][i][in.PatientAgeGroup(p)]==0)
        min_max[oar][i].second=NewMax(oar,i);
    }
  }
  if(nad==-1)
    scheduled--;
  else{
    surgeon_day_load[as][nad]+=in.PatientSurgeryDuration(p);
    operatingtheater_day_load[naot][nad]+=in.PatientSurgeryDuration(p);
    surgeon_day_theater_count[as][nad][naot]++;
    if(surgeon_day_theater_count[as][nad][naot]==1)
      surgeon_day_theaters_visited[as][nad]++;
    operatingtheater_day_patient_list[naot][nad].emplace_back(p);
    daily_admissions[nad].emplace_back(p);
    int rel_sh;
    for(i=nad;i<min(nad+in.PatientLengthOfStay(p),in.Days());i++){ 
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        an=NurseAttending(nar,sh);
        rel_sh=sh-(nad*in.ShiftsPerDay());
        nurse_shift_load[an][sh]+=in.PatientWorkloadProduced(p,rel_sh);
        ever_assigned[p][an]++;
        if(ever_assigned[p][an]==1)
          patient_num_nurses[p]++;
      }
      if(in.PatientGender(p)==Gender::A)
        room_day_a_patients[nar][i]++;
      else
        room_day_b_patients[nar][i]++;
      room_day_patient_list[nar][i].emplace_back(p);
      room_day_age_distribution[nar][i][in.PatientAgeGroup(p)]++;
      if(min_max[nar][i].first==-1 && min_max[nar][i].second==-1){
        min_max[nar][i].first=in.PatientAgeGroup(p);
        min_max[nar][i].second=in.PatientAgeGroup(p);
      }
      else if(min_max[nar][i].first>in.PatientAgeGroup(p))
        min_max[nar][i].first=in.PatientAgeGroup(p);
      else if(min_max[nar][i].second<in.PatientAgeGroup(p))
        min_max[nar][i].second=in.PatientAgeGroup(p);
    }
  }
}

void IH_Output::ChangeOT(int p,int n_ot){
  int o_ot=operating_room[p],s=in.PatientSurgeon(p),du=in.PatientSurgeryDuration(p),d=admission_day[p];
  operating_room[p]=n_ot;
  operatingtheater_day_patient_list[n_ot][d].emplace_back(p);
  int i=FindPatientinOT(p,o_ot,d);
  operatingtheater_day_patient_list[o_ot][d].erase(operatingtheater_day_patient_list[o_ot][d].begin()+i);
  surgeon_day_theater_count[s][d][o_ot]--;
  if(surgeon_day_theater_count[s][d][o_ot]==0)
    surgeon_day_theaters_visited[s][d]--;
  surgeon_day_theater_count[s][d][n_ot]++;
  if(surgeon_day_theater_count[s][d][n_ot]==1)
    surgeon_day_theaters_visited[s][d]++;
  operatingtheater_day_load[o_ot][d]-=du;
  operatingtheater_day_load[n_ot][d]+=du;
}

void IH_Output::ChangeRoom(int p,int nr){
  int o_r=room[p],d=admission_day[p],i,j,sh,rel_sh,an1,an2;
  room[p]=nr;
    for(i=d;i<min(d+in.PatientLengthOfStay(p),in.Days());i++){
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        an1=NurseAttending(o_r,sh);
        an2=NurseAttending(nr,sh);
        if(an1!=an2){
          rel_sh=sh-(d*in.ShiftsPerDay());
          nurse_shift_load[an1][sh]-=in.PatientWorkloadProduced(p,rel_sh);
          nurse_shift_load[an2][sh]+=in.PatientWorkloadProduced(p,rel_sh);
          ever_assigned[p][an1]--;
          ever_assigned[p][an2]++;
          if(ever_assigned[p][an1]==0)
            patient_num_nurses[p]--;
          if(ever_assigned[p][an2]==1)
            patient_num_nurses[p]++;
        }
      }
      if(in.PatientGender(p)==Gender::A){
        room_day_a_patients[o_r][i]--;
        room_day_a_patients[nr][i]++;
      }
      else{
        room_day_b_patients[o_r][i]--;
        room_day_b_patients[nr][i]++;
      }
      j=FindPatientinRoom(p,o_r,i);
      room_day_patient_list[o_r][i].erase(room_day_patient_list[o_r][i].begin()+j);
      room_day_patient_list[nr][i].emplace_back(p);
      room_day_age_distribution[o_r][i][in.PatientAgeGroup(p)]--;
      room_day_age_distribution[nr][i][in.PatientAgeGroup(p)]++;
      if(min_max[o_r][i].first==in.PatientAgeGroup(p) && room_day_age_distribution[o_r][i][in.PatientAgeGroup(p)]==0)
        min_max[o_r][i].first=NewMin(o_r,i);
      if(min_max[o_r][i].second==in.PatientAgeGroup(p) && room_day_age_distribution[o_r][i][in.PatientAgeGroup(p)]==0)
        min_max[o_r][i].second=NewMax(o_r,i);
      if(min_max[nr][i].first==-1 && min_max[nr][i].second==-1){
        min_max[nr][i].first=in.PatientAgeGroup(p);
        min_max[nr][i].second=in.PatientAgeGroup(p);
      }
      if(min_max[nr][i].first>in.PatientAgeGroup(p)){
        min_max[nr][i].first=in.PatientAgeGroup(p);
      }
      if(min_max[nr][i].second<in.PatientAgeGroup(p)){
        min_max[nr][i].second=in.PatientAgeGroup(p);
      }
    }
}

void IH_Output::ChangeNurse(int s, int r, int nn){
  int on=room_shift_nurse[r][s],d=s/in.ShiftsPerDay(),p,rel_sh,i,j;
  room_shift_nurse[r][s]=nn;
  j=FindRoominNurseList(r,on,s);
  nurse_shift_room_list[on][s].erase(nurse_shift_room_list[on][s].begin()+j);
  nurse_shift_room_list[nn][s].emplace_back(r);
  for(i=0;i<room_day_patient_list[r][d].size();i++){
    p=room_day_patient_list[r][d][i];
    ever_assigned[p][on]--;
    ever_assigned[p][nn]++;
    if(ever_assigned[p][on]==0)
      patient_num_nurses[p]--;
    if(ever_assigned[p][nn]==1)
      patient_num_nurses[p]++;
    if(p<in.Patients()){
      rel_sh=s-(admission_day[p]*in.ShiftsPerDay());
      nurse_shift_load[on][s]-=in.PatientWorkloadProduced(p,rel_sh);
      nurse_shift_load[nn][s]+=in.PatientWorkloadProduced(p,rel_sh);
    }
    else{
      nurse_shift_load[on][s]-=in.OccupantWorkloadProduced(p-in.Patients(),s);
      nurse_shift_load[nn][s]+=in.OccupantWorkloadProduced(p-in.Patients(),s);
    }
  }
}

void IH_Output::SwapPatients(int p1,int p2){
  int d1=admission_day[p1],d2=admission_day[p2];
  if(d1>d2){
    swap(d1,d2);
    swap(p1,p2);
  }
  if(d1==-1)
    SwapInOutPatients(p1,p2);
  else{
    if(d1!=d2){ 
      daily_admissions[d2][FindPatientinDailyAdmission(p2,d2)]=p1;
      daily_admissions[d1][FindPatientinDailyAdmission(p1,d1)]=p2;
    }
    SwapOTPatients(p1,p2);
    SwapRoomsPatients(p1,p2);
    SwapNursesPatients(p1,p2);
    swap(admission_day[p1], admission_day[p2]);
    swap(room[p1], room[p2]);
    swap(operating_room[p1], operating_room[p2]);
  }
}


void IH_Output::SwapInOutPatients(int p1,int p2){
  int d=admission_day[p2],s1=in.PatientSurgeon(p1),s2=in.PatientSurgeon(p2),ot=operating_room[p2],r=room[p2];
  int st_min=min(in.PatientLengthOfStay(p1),in.PatientLengthOfStay(p2)),st_max=max(in.PatientLengthOfStay(p1),in.PatientLengthOfStay(p2));
  int i,j,n,sh,rel_sh;
  bool add=in.PatientLengthOfStay(p1)>in.PatientLengthOfStay(p2)? true:false;
  swap(admission_day[p1], admission_day[p2]);
  swap(room[p1], room[p2]);
  swap(operating_room[p1], operating_room[p2]);
  daily_admissions[d][FindPatientinDailyAdmission(p2,d)]=p1; 
  operatingtheater_day_patient_list[ot][d][FindPatientinOT(p2,ot,d)]=p1;
  operatingtheater_day_load[ot][d]+=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
  if(s1!=s2){
    surgeon_day_load[s2][d]-=in.PatientSurgeryDuration(p2);
    surgeon_day_load[s1][d]+=in.PatientSurgeryDuration(p1);
    surgeon_day_theater_count[s2][d][ot]--;
    surgeon_day_theater_count[s1][d][ot]++;
    if(surgeon_day_theater_count[s2][d][ot]==0)
      surgeon_day_theaters_visited[s2][d]--;
    if(surgeon_day_theater_count[s1][d][ot]==1)
      surgeon_day_theaters_visited[s1][d]++;
  }
  else
    surgeon_day_load[s1][d]+=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
  for(i=d;i<min(d+st_min,in.Days());i++){
    room_day_patient_list[r][i][FindPatientinRoom(p2,r,i)]=p1;
    if(in.PatientAgeGroup(p1)!=in.PatientAgeGroup(p2)){
      room_day_age_distribution[r][i][in.PatientAgeGroup(p2)]--;
      room_day_age_distribution[r][i][in.PatientAgeGroup(p1)]++;
      if(RoomDayLoad(r,i)==1)
        min_max[r][i].second=min_max[r][i].first=in.PatientAgeGroup(p1);
      else{
        if(min_max[r][i].first==in.PatientAgeGroup(p2) && room_day_age_distribution[r][i][in.PatientAgeGroup(p2)]==0)
          min_max[r][i].first=NewMin(r,i);
        else if(min_max[r][i].second==in.PatientAgeGroup(p2) && room_day_age_distribution[r][i][in.PatientAgeGroup(p2)]==0)
          min_max[r][i].second=NewMax(r,i);
        if(in.PatientAgeGroup(p1)<min_max[r][i].first)
          min_max[r][i].first=in.PatientAgeGroup(p1);
        else if(in.PatientAgeGroup(p1)>min_max[r][i].second)
          min_max[r][i].second=in.PatientAgeGroup(p1);
      }
    }
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(d*in.ShiftsPerDay());
        n=NurseAttending(r,sh);
        nurse_shift_load[n][sh]+=in.PatientWorkloadProduced(p1,rel_sh)-in.PatientWorkloadProduced(p2,rel_sh);
        ever_assigned[p1][n]++;
        if(ever_assigned[p1][n]==1)
          patient_num_nurses[p1]++;
      }
    if(in.PatientGender(p1)!=in.PatientGender(p2)){
      if(in.PatientGender(p2)==Gender::B){
        room_day_b_patients[r][i]--;
        room_day_a_patients[r][i]++;
      }
      else{
        room_day_b_patients[r][i]++;
        room_day_a_patients[r][i]--;
      }
    }
  }
  for(i=d+st_min;i<min(d+st_max,in.Days());i++){ 
    if(add){ 
      room_day_patient_list[r][i].emplace_back(p1);
      room_day_age_distribution[r][i][in.PatientAgeGroup(p1)]++;
      if(in.PatientGender(p1)==Gender::A)
        room_day_a_patients[r][i]++;
      else
        room_day_b_patients[r][i]++;
      if(RoomDayLoad(r,i)==1) 
        min_max[r][i].second=min_max[r][i].first=in.PatientAgeGroup(p1);
      else{ 
        if(min_max[r][i].first>in.PatientAgeGroup(p1))
          min_max[r][i].first=in.PatientAgeGroup(p1);
        else if (min_max[r][i].second<in.PatientAgeGroup(p1))
          min_max[r][i].second=in.PatientAgeGroup(p1);
      }
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(d*in.ShiftsPerDay());
        n=NurseAttending(r,sh);
        nurse_shift_load[n][sh]+=in.PatientWorkloadProduced(p1,rel_sh);
        ever_assigned[p1][n]++;
        if(ever_assigned[p1][n]==1)
          patient_num_nurses[p1]++;
      }
    }
    else{ 
      room_day_patient_list[r][i].erase(room_day_patient_list[r][i].begin()+FindPatientinRoom(p2,r,i));
      room_day_age_distribution[r][i][in.PatientAgeGroup(p2)]--;
      if(in.PatientGender(p2)==Gender::A)
        room_day_a_patients[r][i]--;
      else
        room_day_b_patients[r][i]--;
      if(RoomDayLoad(r,i)==0)
        min_max[r][i].second=min_max[r][i].first=-1;
      else if(room_day_age_distribution[r][i][in.PatientAgeGroup(p2)]==0){ 
        if(min_max[r][i].first==in.PatientAgeGroup(p2))
          min_max[r][i].first=NewMin(r,i);
        else if(min_max[r][i].second==in.PatientAgeGroup(p2))
          min_max[r][i].second=NewMax(r,i);
      }

      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        rel_sh=sh-(d*in.ShiftsPerDay());
        n=NurseAttending(r,sh);
        nurse_shift_load[n][sh]-=in.PatientWorkloadProduced(p2,rel_sh);
      }
    }
  }

  patient_num_nurses[p2]=0;
  for(i=0;i<in.Nurses();i++)
    ever_assigned[p2][i]=0;
}


void IH_Output::SwapOTPatients(int p1,int p2){
  int d1=admission_day[p1],d2=admission_day[p2],ot1=operating_room[p1],ot2=operating_room[p2];
  int as1=in.PatientSurgeon(p1),as2=in.PatientSurgeon(p2);
  if(as1==as2){
      surgeon_day_load[as1][d1]+=in.PatientSurgeryDuration(p2)-in.PatientSurgeryDuration(p1);
      surgeon_day_load[as1][d2]+=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
    }
    else{
      surgeon_day_load[as1][d1]-=in.PatientSurgeryDuration(p1);
      surgeon_day_load[as2][d1]+=in.PatientSurgeryDuration(p2);
      surgeon_day_load[as1][d2]+=in.PatientSurgeryDuration(p1);
      surgeon_day_load[as2][d2]-=in.PatientSurgeryDuration(p2);
    }
    if(surgeon_day_theater_count[as1][d1][ot1]==1)
      surgeon_day_theaters_visited[as1][d1]--;    
    surgeon_day_theater_count[as1][d1][ot1]--;
    if(surgeon_day_theater_count[as2][d1][ot1]==0)
      surgeon_day_theaters_visited[as2][d1]++;    
    surgeon_day_theater_count[as2][d1][ot1]++;
    if(surgeon_day_theater_count[as2][d2][ot2]==1)
      surgeon_day_theaters_visited[as2][d2]--;
    surgeon_day_theater_count[as2][d2][ot2]--;
    if(surgeon_day_theater_count[as1][d2][ot2]==0)
      surgeon_day_theaters_visited[as1][d2]++;
    surgeon_day_theater_count[as1][d2][ot2]++;
    operatingtheater_day_load[ot1][d1]-=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
    operatingtheater_day_load[ot2][d2]+=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
    operatingtheater_day_patient_list[ot1][d1][FindPatientinOT(p1,ot1,d1)]=p2;
    operatingtheater_day_patient_list[ot2][d2][FindPatientinOT(p2,ot2,d2)]=p1;
}

void IH_Output::SwapRoomsPatients(int p1, int p2){
  int d1=admission_day[p1],d2=admission_day[p2];
  int r1=room[p1],r2=room[p2];
  for(int i=d1;i<min(d1+in.PatientLengthOfStay(p1),in.Days());i++){
    if(in.PatientGender(p1) == Gender::A)
      room_day_a_patients[r1][i]--;
    else
      room_day_b_patients[r1][i]--;
    room_day_age_distribution[r1][i][in.PatientAgeGroup(p1)]--;
    if(room_day_age_distribution[r1][i][in.PatientAgeGroup(p1)]==0){
      if(min_max[r1][i].first==in.PatientAgeGroup(p1))
        min_max[r1][i].first=NewMin(r1,i);
      if(min_max[r1][i].second==in.PatientAgeGroup(p1))
        min_max[r1][i].second=NewMax(r1,i);
    }
    room_day_patient_list[r1][i].erase(room_day_patient_list[r1][i].begin()+FindPatientinRoom(p1,r1,i));
  }
  for(int i=d2;i<min(d2+in.PatientLengthOfStay(p2),in.Days());i++){
    if(in.PatientGender(p2)== Gender::A)
      room_day_a_patients[r2][i]--;
    else
      room_day_b_patients[r2][i]--;
    room_day_age_distribution[r2][i][in.PatientAgeGroup(p2)]--;
    room_day_patient_list[r2][i].erase(room_day_patient_list[r2][i].begin()+FindPatientinRoom(p2,r2,i));
    if(room_day_age_distribution[r2][i][in.PatientAgeGroup(p2)]==0){
      if(min_max[r2][i].first==in.PatientAgeGroup(p2))
        min_max[r2][i].first=NewMin(r2,i);
      if(min_max[r2][i].second==in.PatientAgeGroup(p2))
        min_max[r2][i].second=NewMax(r2,i);
    }
  }
  for(int i=d1;i<min(d1+in.PatientLengthOfStay(p2),in.Days());i++){
    if(in.PatientGender(p2)== Gender::A)
      room_day_a_patients[r1][i]++;
    else
      room_day_b_patients[r1][i]++;
    room_day_age_distribution[r1][i][in.PatientAgeGroup(p2)]++;
    if(room_day_age_distribution[r1][i][in.PatientAgeGroup(p2)]==1){
      if(min_max[r1][i].first==-1 && min_max[r1][i].second==-1){
        min_max[r1][i].first=in.PatientAgeGroup(p2);
        min_max[r1][i].second=in.PatientAgeGroup(p2);
      }
      if(min_max[r1][i].first>in.PatientAgeGroup(p2))
        min_max[r1][i].first=in.PatientAgeGroup(p2);
      if(min_max[r1][i].second<in.PatientAgeGroup(p2))
        min_max[r1][i].second=in.PatientAgeGroup(p2);
    }
    room_day_patient_list[r1][i].emplace_back(p2);
  }
  for(int i=d2;i<min(d2+in.PatientLengthOfStay(p1),in.Days());i++){
    if(in.PatientGender(p1) == Gender::A)
      room_day_a_patients[r2][i]++;
    else
      room_day_b_patients[r2][i]++;
    room_day_age_distribution[r2][i][in.PatientAgeGroup(p1)]++;
    room_day_patient_list[r2][i].emplace_back(p1);
    if(room_day_age_distribution[r2][i][in.PatientAgeGroup(p1)]==1){
      if(min_max[r2][i].first==-1 && min_max[r2][i].second==-1){
        min_max[r2][i].first=in.PatientAgeGroup(p1);
        min_max[r2][i].second=in.PatientAgeGroup(p1);
      }
      if(min_max[r2][i].first>in.PatientAgeGroup(p1))
        min_max[r2][i].first=in.PatientAgeGroup(p1);
      if(min_max[r2][i].second<in.PatientAgeGroup(p1))
        min_max[r2][i].second=in.PatientAgeGroup(p1);
    }
  }
}

void IH_Output::SwapNursesPatients(int p1,int p2){       
  int d1=admission_day[p1],d2=admission_day[p2];
  int r1=room[p1],r2=room[p2],i,j,idx,rel_sh,cn;    
  for(i=d1;i<d1+in.PatientLengthOfStay(p1);i++){
   for(j=0;j<in.ShiftsPerDay();j++){
     idx=(i*in.ShiftsPerDay())+j;
     if(i<in.Days()){
       cn=NurseAttending(r1,idx);
       ever_assigned[p1][cn]--;
       if(ever_assigned[p1][cn]==0)
         patient_num_nurses[p1]--;
       nurse_shift_load[cn][idx]-=in.PatientWorkloadProduced(p1,idx-(d1*in.ShiftsPerDay()));
     }
   }
  }
  for(i=d2;i<d2+in.PatientLengthOfStay(p2);i++){
   for(j=0;j<in.ShiftsPerDay();j++){
     idx=(i*in.ShiftsPerDay())+j;
     if(i<in.Days()){
       cn=NurseAttending(r2,idx);
       ever_assigned[p2][cn]--;
       if(ever_assigned[p2][cn]==0)
         patient_num_nurses[p2]--;
       nurse_shift_load[cn][idx]-=in.PatientWorkloadProduced(p2,idx-(d2*in.ShiftsPerDay()));
     }
   }
  }
  for(i=d1;i<d1+in.PatientLengthOfStay(p2);i++){
    for(j=0;j<in.ShiftsPerDay();j++){
      idx=(i*in.ShiftsPerDay())+j;
      if(i<in.Days()){
        rel_sh=idx-(d1*in.ShiftsPerDay()),cn=NurseAttending(r1,idx);
        ever_assigned[p2][cn]++;
        if(ever_assigned[p2][cn]==1)
          patient_num_nurses[p2]++;
        nurse_shift_load[cn][idx]+=in.PatientWorkloadProduced(p2,rel_sh);
      }
    }
  }
  for(i=d2;i<d2+in.PatientLengthOfStay(p1);i++){
    for(int j=0;j<in.ShiftsPerDay();j++){
      idx=(i*in.ShiftsPerDay())+j;
      if(i<in.Days()){
        rel_sh=idx-(d2*in.ShiftsPerDay()),cn=NurseAttending(r2,idx);
        ever_assigned[p1][cn]++;
        if(ever_assigned[p1][cn]==1)
          patient_num_nurses[p1]++;
        nurse_shift_load[cn][idx]+=in.PatientWorkloadProduced(p1,rel_sh);
      }
    }
  }
}

int IH_Output::FindPatientinOT(int p, int ot, int ad) const{
  for(int i=0;i<operatingtheater_day_patient_list[ot][ad].size();i++){
    if(operatingtheater_day_patient_list[ot][ad][i]==p)
      return i; 
  }
  throw invalid_argument ("patient does not seem to be in OT ");
}

int IH_Output::FindPatientinRoom(int p, int r, int d) const{
  for(int i=0;i<room_day_patient_list[r][d].size();i++){
    if(room_day_patient_list[r][d][i]==p)
      return i; 
  }
  throw invalid_argument("patient " + to_string(p) + " does not seem to be in room "+to_string(r)+" on day " + to_string(d));
}

int IH_Output::FindRoominNurseList(int r, int n, int s) const{
  for(int i=0;i<nurse_shift_room_list[n][s].size();i++){
    if(nurse_shift_room_list[n][s][i]==r)
      return i; 
  }
  throw invalid_argument ("room" + to_string(r) + "does not seem to be in nurse list ");
}

int IH_Output::FindPatientinDailyAdmission(int p, int d) const{
  for(int i=0;i<daily_admissions[d].size();i++){
    if(daily_admissions[d][i]==p)
      return i; 
  }
  throw invalid_argument ("patient" + to_string(p) + "does not seem to in the admission list of day " + to_string(d));  
}

vector<int> IH_Output::FindOverlappingNurses(int p,int nad,int n_r) const{
  vector<int> overlap_shifts;
  int oad=admission_day[p],st=in.PatientLengthOfStay(p),o_r=room[p],sh;
  if(oad!=-1 && nad!=-1){
    if(oad==nad){
      for(int i=nad;i<min(nad+st,in.Days());i++){
        for(int j=0;j<in.ShiftsPerDay();j++){
          sh=i*in.ShiftsPerDay()+j;
          if(room_shift_nurse[o_r][sh]==room_shift_nurse[n_r][sh])
            overlap_shifts.emplace_back(sh);
        }
      }
    }
    else if(oad<nad && oad+st>nad){
      for(int i=nad;i<min(oad+st,in.Days());i++)
        for(int j=0;j<in.ShiftsPerDay();j++){
          sh=i*in.ShiftsPerDay()+j;
          if(room_shift_nurse[o_r][sh]==room_shift_nurse[n_r][sh])
            overlap_shifts.emplace_back(sh);
        }
    }
    else if(oad>nad && nad+st>oad){
      for(int i=oad;i<min(nad+st,in.Days());i++)
        for(int j=0;j<in.ShiftsPerDay();j++){
          sh=i*in.ShiftsPerDay()+j;
        if(room_shift_nurse[o_r][sh]==room_shift_nurse[n_r][sh])
          overlap_shifts.emplace_back(sh);
      }
    }
  }
  return overlap_shifts;
}

vector<int> IH_Output::FindCurrentOverlappingNurses(int p1,int p2) const{
  vector<int> overlap_shifts;
  int d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2),r1=room[p1],r2=room[p2];
  int sh,i,j;
  int ed1=min(d1+st1,in.Days()),ed2=min(d2+st2,in.Days());
  if(ed1>d2){
    for(i=d2;i<min(ed1,ed2);i++){
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(room_shift_nurse[r1][sh]==room_shift_nurse[r2][sh])
          overlap_shifts.emplace_back(sh);
      }
    }
  }
  return overlap_shifts;
}

vector<int> IH_Output::FindFutureOverlappingNurses(int p1,int p2) const{
  vector<int> overlap_shifts;
  int d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2),r1=room[p1],r2=room[p2];
  int sh,i,j;
  int fed1=min(d1+st2,in.Days()),fed2=min(d2+st1,in.Days());
  if(fed1>d2){
    for(i=d2;i<min(fed2,fed1);i++){
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(room_shift_nurse[r1][sh]==room_shift_nurse[r2][sh])
          overlap_shifts.emplace_back(sh);
      }
    }
  }
  return overlap_shifts;
}

int IH_Output::NewMin(int r,int d) const{
  for(int i=0;i<in.AgeGroups();i++)
    if(room_day_age_distribution[r][d][i]>0)
      return i;
  return -1;
}

int IH_Output::NewMax(int r,int d) const{
  for(int i=in.AgeGroups()-1;i>=0;i--){
    if(room_day_age_distribution[r][d][i]>0)
      return i;
  }
  return -1;
}

int IH_Output::NewProjMin(int r,int d) const{
  bool found=false;
  for(int i=0;i<in.AgeGroups();i++){
    if(!found && room_day_age_distribution[r][d][i])
      found=true;
    else if(found && room_day_age_distribution[r][d][i]>0)
      return i;
  }
  return -1;
}

int IH_Output::NewProjMax(int r,int d) const{
  bool found=false;
  for(int i=in.AgeGroups()-1;i>=0;i--){
    if(!found && room_day_age_distribution[r][d][i]>0)
      found=true;
    else if(found && room_day_age_distribution[r][d][i]>0)
      return i;
  }
  return -1;
}

//Counters

int IH_Output::CountDistinctNurses(int p) const
{
	return count_if(ever_assigned[p].begin(),ever_assigned[p].end(),[](int x){return x>0;});
}

int IH_Output::CountOccupantNurses(int o) const
{
  return count_if(ever_assigned[o+in.Patients()].begin(),ever_assigned[o+in.Patients()].end(),[](int x){return x>0;});
}

bool IH_Output::TwoOTOpen(int d) const{
  int count = 0;
  for(int i = 0; i < in.OperatingTheaters(); i++) {
    if(OperatingTheaterDayLoad(d, i) > 0 && ++count >= 2)
      return true;
  }
  return false;
}

//Deltas

int IH_Output::InOutDeltaOvertimeSurgeon(int p1,int p2) const{
  if(admission_day[p2]==-1)
    swap(p1,p2);
  int delta=0,d2=admission_day[p2],s1=in.PatientSurgeon(p1),s2=in.PatientSurgeon(p2),diff;
  if(s1==s2){ 
    diff=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
    if(surgeon_day_load[s1][d2]+diff>in.SurgeonMaxSurgeryTime(s1,d2)){
      if(surgeon_day_load[s1][d2]>in.SurgeonMaxSurgeryTime(s1,d2))
        delta+=diff;
      else
        delta+=surgeon_day_load[s1][d2]+diff-in.SurgeonMaxSurgeryTime(s1,d2);
    }
  }
  else{ 
    diff=in.PatientSurgeryDuration(p1);
    if(surgeon_day_load[s1][d2]+diff>in.SurgeonMaxSurgeryTime(s1,d2)){
      if(surgeon_day_load[s1][d2]>in.SurgeonMaxSurgeryTime(s1,d2))
        delta+=diff;
      else
        delta+=surgeon_day_load[s1][d2]+diff-in.SurgeonMaxSurgeryTime(s1,d2);
    }
    diff=in.PatientSurgeryDuration(p2);
    if(surgeon_day_load[s2][d2]>in.SurgeonMaxSurgeryTime(s2,d2)){
      if(surgeon_day_load[s2][d2]-diff>in.SurgeonMaxSurgeryTime(s2,d2))
        delta-=diff;
      else
        delta-=surgeon_day_load[s2][d2]-in.SurgeonMaxSurgeryTime(s2,d2);
    }
  }
  return delta;
}

int IH_Output::InOutDeltaOvertimeOT(int p1,int p2) const{
  if(admission_day[p2]==-1)
    swap(p1,p2);
  int delta=0,d2=admission_day[p2],ot=operating_room[p2],diff=in.PatientSurgeryDuration(p1)-in.PatientSurgeryDuration(p2);
  if(OperatingTheaterDayLoad(ot,d2)>in.OperatingTheaterAvailability(ot,d2)){
    if(OperatingTheaterDayLoad(ot,d2)+diff>in.OperatingTheaterAvailability(ot,d2))
      delta+=diff;
    else
      delta+=OperatingTheaterDayLoad(ot,d2)-in.OperatingTheaterAvailability(ot,d2);
  }
  else if(OperatingTheaterDayLoad(ot,d2)+diff>in.OperatingTheaterAvailability(ot,d2))
    delta+=OperatingTheaterDayLoad(ot,d2)+diff-in.OperatingTheaterAvailability(ot,d2);
  return delta;
}

int IH_Output::DeltaOpenOT(int p,int n_d,int n_ot) const{
  int delta=0;
  int oad=admission_day[p],o_ot=operating_room[p],d=in.PatientSurgeryDuration(p);
  if(oad!=-1 && OperatingTheaterDayLoad(o_ot,oad)==d)
    delta--;
  if(n_ot!=-1 && OperatingTheaterDayLoad(n_ot,n_d)==0)
    delta++; 
  return delta;
}

int IH_Output::DeltaSurgeonOvertimeMP(int p,int nd) const{
  int delta=0;
  int od=admission_day[p],d=in.PatientSurgeryDuration(p),s=in.PatientSurgeon(p);
  if(od!=-1 && SurgeonDayLoad(s,od)>in.SurgeonMaxSurgeryTime(s,od)){
    if(SurgeonDayLoad(s,od)-d >in.SurgeonMaxSurgeryTime(s,od))
      delta-=d;
    else
      delta-=SurgeonDayLoad(s,od)-in.SurgeonMaxSurgeryTime(s,od);
  }
  if(nd!=-1 && SurgeonDayLoad(s,nd)+d>in.SurgeonMaxSurgeryTime(s,nd)){
    if(SurgeonDayLoad(s,nd)>in.SurgeonMaxSurgeryTime(s,nd))
      delta+=d;
    else
      delta+=SurgeonDayLoad(s,nd)+d-in.SurgeonMaxSurgeryTime(s,nd);
  }
  return delta;
}

int IH_Output::NotOverlappingSwapOvercapacity(int p1,int p2) const{
  int delta=0,i;
  int d1=admission_day[p1],d2=admission_day[p2],r1=room[p1],r2=room[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  
  if(r1==r2 && d1==d2)
    return delta;
  if(st1>st2){ 
    for(i=fed2;i<ed1;i++)
      if(RoomDayLoad(r1,i)>in.RoomCapacity(r1))
        delta--;
    for(i=ed2;i<fed1;i++)
      if(RoomDayLoad(r2,i)>=in.RoomCapacity(r2))
        delta++;
  }
  else{ 
    for(i=ed1;i<fed2;i++)
      if(RoomDayLoad(r1,i)>=in.RoomCapacity(r1))
        delta++;
    for(i=fed1;i<ed2;i++)
      if(RoomDayLoad(r2,i)>in.RoomCapacity(r2))
        delta--;
  }
  return delta;
}

int IH_Output::OverlappingSwapOvercapacity(int p1,int p2) const{
  
  int delta=0,i;
  int d1=admission_day[p1],d2=admission_day[p2],r1=room[p1],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  bool olp=((d1<d2 && (d2<ed1)) || (d2<d1 && (d1<ed2)));
  bool folp=((d1<d2 && (d2<fed2)) || (d2<d1 && (d1<fed1)));
  int p_left,p_right,st_left,st_right;
  
  if(d1<d2){
   p_left=p1;
   p_right=p2;
   st_left=st1;
   st_right=st2;
  }
  else{
    p_left=p2;
    p_right=p1;
    st_left=st2;
    st_right=st1;
  }
 int left_end=min(admission_day[p_left]+st_left,in.Days()), left_fend=min(admission_day[p_left]+st_right,in.Days());
 int right_end=min(admission_day[p_right]+st_right,in.Days()), right_fend=min(admission_day[p_right]+st_left,in.Days());
 if(olp && !folp){ 
  for(i=left_fend;i<min(left_end,right_end);i++)
    if(RoomDayLoad(r1,i)>in.RoomCapacity(r1))
      delta--;
  for(i=max(right_end,left_end);i<right_fend;i++)
    if(RoomDayLoad(r1,i)>=in.RoomCapacity(r1))
      delta++;
 }
 else if(olp && folp){
  if(st_left>st_right){ 
    for(i=left_fend;i<min(left_end,right_end);i++) 
      if(RoomDayLoad(r1,i)>in.RoomCapacity(r1))
        delta--;
    for(i=max(right_end,left_end);i<right_fend;i++){
      if(RoomDayLoad(r1,i)>=in.RoomCapacity(r1))
        delta++;
    }
  }
  else{
    for(i=left_end;i<min(left_fend,right_fend);i++) 
      if(RoomDayLoad(r1,i)>=in.RoomCapacity(r1))
        delta++;
    for(i=max(right_fend,left_fend);i<right_end;i++) 
      if(RoomDayLoad(r1,i)>in.RoomCapacity(r1))
        delta--;
  }
 }
 else{
  for(i=left_end;i<min(left_fend,right_fend);i++)
    if(RoomDayLoad(r1,i)>=in.RoomCapacity(r1))
      delta++;
  for(i=max(right_fend,left_fend);i<right_end;i++)
    if(RoomDayLoad(r1,i)>in.RoomCapacity(r1))
      delta--;
 }
  return delta;
}

int IH_Output::NotOverlappingSwapGenderMix(int p1,int p2) const{
  int delta=0,i;
  int d1=admission_day[p1],d2=admission_day[p2],r1=room[p1],r2=room[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  Gender g1=in.PatientGender(p1),g2=in.PatientGender(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  if(r1==r2 && d1==d2)
    return delta;
  if(g1!=g2){ 
  for(i=d1;i<min(ed1,fed2);i++){
    if(RoomDayLoad(r1,i)>1){
      if(g1==Gender::A){
        if(RoomDayBPatients(r1,i)>=RoomDayAPatients(r1,i))
          delta--;
        else if(RoomDayBPatients(r1,i)<RoomDayAPatients(r1,i)){
          if(RoomDayBPatients(r1,i)==0 && RoomDayAPatients(r1,i)>1){ 
            delta++;
          }
          else if(RoomDayAPatients(r1,i)==1 && RoomDayBPatients(r1,i)>0) 
            delta--;
          else if(RoomDayBPatients(r1,i)+1>=RoomDayAPatients(r1,i)-1) 
            delta+=(RoomDayAPatients(r1,i)-1)-RoomDayBPatients(r1,i);     
          else
            delta++;
        }
      }
      else{   
        if(RoomDayAPatients(r1,i)>=RoomDayBPatients(r1,i))
          delta--;
        else if(RoomDayAPatients(r1,i)<RoomDayBPatients(r1,i)){
          if(RoomDayAPatients(r1,i)==0 && RoomDayBPatients(r1,i)>1) 
            delta++;
          else if(RoomDayBPatients(r1,i)==1 && RoomDayAPatients(r1,i)>0) 
            delta--;
          else if(RoomDayAPatients(r1,i)+1>=RoomDayBPatients(r1,i)-1) 
            delta+=(RoomDayBPatients(r1,i)-1)-RoomDayAPatients(r1,i);     
          else
            delta++;
        }
      }
    }
  }
  for(i=d2;i<min(ed2,fed1);i++){
    if(RoomDayLoad(r2,i)>1){
      if(in.PatientGender(p2)==Gender::A){
        if(RoomDayBPatients(r2,i)>=RoomDayAPatients(r2,i))
          delta--;
        else if(RoomDayBPatients(r2,i)<RoomDayAPatients(r2,i)){
          if(RoomDayBPatients(r2,i)==0 && RoomDayAPatients(r2,i)>1) 
            delta++;
          else if(RoomDayAPatients(r2,i)==1 && RoomDayBPatients(r2,i)>0) 
            delta--;
          else if(RoomDayBPatients(r2,i)+1>=RoomDayAPatients(r2,i)-1) 
            delta+=(RoomDayAPatients(r2,i)-1)-RoomDayBPatients(r2,i);     
          else
            delta++;
        }
      }
      else{ 
        if(RoomDayAPatients(r2,i)>=RoomDayBPatients(r2,i))
          delta--;
        else if(RoomDayAPatients(r2,i)<RoomDayBPatients(r2,i)){
          if(RoomDayAPatients(r2,i)==0 && RoomDayBPatients(r2,i)>1) 
            delta++;
          else if(RoomDayBPatients(r2,i)==1 && RoomDayAPatients(r2,i)>0) 
            delta--;
          else if(RoomDayAPatients(r2,i)+1>=(RoomDayBPatients(r2,i)-1)) 
            delta+=(RoomDayBPatients(r2,i)-1)-RoomDayAPatients(r2,i);   
          else
            delta++;
        }
      }
    }
  }
  }
  if(st1>st2){
    for(i=fed2;i<ed1;i++){ 
      if(g1==Gender::A){
        if(RoomDayAPatients(r1,i)<=RoomDayBPatients(r1,i))
          delta--;
      }
      else{
        if(RoomDayBPatients(r1,i)<=RoomDayAPatients(r1,i))
          delta--;
      }
    }
    for(i=ed2;i<fed1;i++){
      if(g1==Gender::A){
        if(RoomDayAPatients(r2,i)<RoomDayBPatients(r2,i))
          delta++;
      }
      else{
        if(RoomDayBPatients(r2,i)<RoomDayAPatients(r2,i))
          delta++;
      }
    } 
  }
  else if(st1<st2){
    for(i=ed1;i<fed2;i++){ 
      if(g2==Gender::A){
        if(RoomDayAPatients(r1,i)<RoomDayBPatients(r1,i))
          delta++;
      }
      else{
        if(RoomDayBPatients(r1,i)<RoomDayAPatients(r1,i))
          delta++;
      }
    }
    for(i=fed1;i<ed2;i++){ 
      if(g2==Gender::A){
        if(RoomDayAPatients(r2,i)<=RoomDayBPatients(r2,i))
          delta--;
      }
      else{
        if(RoomDayBPatients(r2,i)<=RoomDayAPatients(r2,i))
          delta--;
      }
    }
  }
  return delta;
}

int IH_Output::OverlappingSwapGenderMix(int p1,int p2) const{
  int delta=0,i;
  int r1=Room(p1),r2=Room(p2),d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  Gender g1=in.PatientGender(p1),g2=in.PatientGender(p2);
  bool olp= (d1<d2 && (d2<ed1)) || (d2<d1 && (d1<ed2));
  bool folp=(d1<d2 && (d2<fed2)) || (d2<d1 && (d1<fed1));
  int p_left,p_right,st_left,st_right,d_left,d_right;
  if(r1==r2 && d1==d2)
    return delta;
  if(d1<d2){
   p_left=p1;
   p_right=p2;
   st_left=st1;
   st_right=st2;
   d_left=d1;
   d_right=d2;
  }
  else{
    p_left=p2;
    p_right=p1;
    st_left=st2;
    st_right=st1;
    d_left=d2;
    d_right=d1;
  }
  int left_end  = min(admission_day[p_left]+st_left,in.Days()),   left_fend  = min(admission_day[p_left]+st_right,in.Days());
  int right_end = min(admission_day[p_right]+st_right,in.Days()), right_fend = min(admission_day[p_right]+st_left,in.Days());
  if(olp && !folp){
    if(g1==g2){
     for(i=left_fend;i<min(left_end,right_end);i++)
       delta+=SubGender(p_left,i);
     for(i=max(right_end,left_end);i<right_fend;i++)
       delta+=AddGender(p_left,i);
    }
    else{
      for(i=d_left;i<left_fend;i++)
        delta+=SwapGenders(p_left,i);
      for(i=left_fend;i<min(left_end,d_right);i++)
        delta+=SubGender(p_left,i);
      for(i=d_right;i<min(left_end,right_end);i++)
        delta+=SubGender(p_right,i);
      for(i=left_end;i<right_end;i++)
        delta+=SwapGenders(p_right,i);
      for(i=max(right_end,left_end);i<right_fend;i++)
        delta+=AddGender(p_left,i);
    }
  }
  else if(olp && folp){
    if(st_left>st_right){ 
      if(g1==g2){
        for(i=left_fend;i<min(left_end,right_end);i++)
          delta+=SubGender(p_left,i);
        for(i=max(right_end,left_end);i<right_fend;i++)
          delta+=AddGender(p_left,i);
      }
      else{
        for(i=d_left;i<d_right;i++)
          delta+=SwapGenders(p_left,i);
        for(i=left_fend;i<min(left_end,right_end);i++)
          delta+=SubGender(p_right,i);
        for(i=left_end;i<right_end;i++)
          delta+=SwapGenders(p_right,i);
        for(i=max(right_end,left_end);i<right_fend;i++)
          delta+=AddGender(p_left,i);
      }
    }
    else{ 
      if(g1==g2){
        for(i=left_end;i<min(left_fend,right_fend);i++)
          delta+=AddGender(p_left,i);
        for(i=max(right_fend,left_fend);i<right_end;i++)
          delta+=SubGender(p_right,i);
      }
      else{
        for(i=d_left;i<d_right;i++)
          delta+=SwapGenders(p_left,i);
        for(i=left_end;i<min(left_fend,right_fend);i++)
          delta+=AddGender(p_left,i);
        for(i=left_fend;i<min(right_fend,right_end);i++)
          delta+=SwapGenders(p_right,i);
        for(i=max(right_fend,left_fend);i<right_end;i++)
          delta+=SubGender(p_right,i);
      }
    }
  }
  else{ 
     if(g1==g2){
      for(i=left_end;i<min(left_fend,right_fend);i++)
        delta+=AddGender(p_left,i);
      for(i=max(right_fend,left_fend);i<right_end;i++){
        delta+=SubGender(p_right,i);
      }
     }
     else{
      for(i=d_left;i<left_end;i++)
        delta+=SwapGenders(p_left,i);
      for(i=left_end;i<d_right;i++)
        delta+=AddGender(p_right,i);
      for(i=d_right;i<min(left_fend,right_fend);i++)
        delta+=AddGender(p_left,i);
      for(i=left_fend;i<right_fend;i++)
        delta+=SwapGenders(p_right,i);
      for(i=max(right_fend,left_fend);i<right_end;i++)
        delta+=SubGender(p_right,i);
     }
  }
  return delta;
}

int IH_Output::NotOverlappingSwapAgeMix(int p1,int p2) const{
  int delta=0,i;
  int d1=admission_day[p1],d2=admission_day[p2],r1=room[p1],r2=room[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  int ag1=in.PatientAgeGroup(p1),ag2=in.PatientAgeGroup(p2);
  int candidate;
  if(r1==r2 && d1==d2)
    return delta;
  if(ag1!=ag2){ 
  for(i=d1;i<min(ed1,fed2);i++){
    if(RoomDayLoad(r1,i)>1){ 
      if(RoomAgePres(r1,i,ag1)>1 && ag1==RoomDayMin(r1,i) && ag1==RoomDayMax(r1,i)) 
        delta+=abs(ag2-ag1);
      else{
        if(ag1==RoomDayMin(r1,i)){ 
          if(ag2<ag1)
            delta+=ag1-ag2;
          else if(RoomAgePres(r1,i,ag1)==1){ 
            candidate=NewProjMin(r1,i);
            delta+=(candidate>ag2)? ag1-ag2:ag1-candidate;
          }
          if(ag2>RoomDayMax(r1,i))
            delta+=ag2-RoomDayMax(r1,i);
        }
        if(ag1==RoomDayMax(r1,i)){ 
          if(ag2>ag1)
            delta+=ag2-ag1;
          else if(RoomAgePres(r1,i,ag1)==1){ 
            candidate=NewProjMax(r1,i);
            delta+=(candidate<ag2)? ag2-ag1:candidate-ag1;
          }
          if(ag2<RoomDayMin(r1,i))
            delta+=RoomDayMin(r1,i)-ag2;
        }
        else if(ag1!=RoomDayMax(r1,i) && ag1!=RoomDayMin(r1,i)){ 
          if(ag2<RoomDayMin(r1,i))
            delta+=RoomDayMin(r1,i)-ag2;
          else if(ag2>RoomDayMax(r1,i))
            delta+=ag2-RoomDayMax(r1,i);
        }
      }
    }
  }
  for(i=d2;i<min(ed2,fed1);i++){
    if(RoomDayLoad(r2,i)>1){ 
      if(RoomAgePres(r2,i,ag2)>1 && ag2==RoomDayMin(r2,i) && ag2==RoomDayMax(r2,i)){
        delta+=abs(ag1-ag2);
      }
      else{
        if(ag2==RoomDayMin(r2,i)){
          if(ag1<ag2)
            delta+=ag2-ag1;
          else if(RoomAgePres(r2,i,ag2)==1){
            candidate=NewProjMin(r2,i);
            delta+=(candidate>ag1)? ag2-ag1:ag2-candidate;
          }
          if(ag1>RoomDayMax(r2,i))
            delta+=ag1-RoomDayMax(r2,i);
        }
        if(ag2==RoomDayMax(r2,i)){
          if(ag1>ag2)
            delta+=ag1-ag2;
          else if(RoomAgePres(r2,i,ag2)==1){
            candidate=NewProjMax(r2,i);
            delta+=(candidate<ag1)? ag1-ag2:candidate-ag2;
          }
          if (ag1<RoomDayMin(r2,i))
            delta+=RoomDayMin(r2,i)-ag1;
        }
        else if(ag2!=RoomDayMax(r2,i) && ag2!=RoomDayMin(r2,i)){
          if(ag1<RoomDayMin(r2,i))
            delta+=RoomDayMin(r2,i)-ag1;
          else if(ag1>RoomDayMax(r2,i))
            delta+=ag1-RoomDayMax(r2,i);
        }
      }
    }
  }
  }
  if(st1>st2){
    for(i=fed2;i<ed1;i++){ 
      if(RoomDayLoad(r1,i)>1 && RoomAgePres(r1,i,ag1)==1){
        if(ag1==RoomDayMin(r1,i))
          delta+=ag1-NewProjMin(r1,i);
        else if(ag1==RoomDayMax(r1,i))
          delta+=NewProjMax(r1,i)-ag1;
      }
    }
    for(i=ed2;i<fed1;i++){ 
      if(RoomDayLoad(r2,i)>0){
        if(ag1<RoomDayMin(r2,i))
          delta+=RoomDayMin(r2,i)-ag1;
        else if(ag1>RoomDayMax(r2,i))
          delta+=ag1-RoomDayMax(r2,i);
      }
    }
  }
  else if(st1<st2){
    for(i=ed1;i<fed2;i++){ 
      if(RoomDayLoad(r1,i)>0){
        if(ag2<RoomDayMin(r1,i))
          delta+=RoomDayMin(r1,i)-ag2;
        else if(ag2>RoomDayMax(r1,i))
          delta+=ag2-RoomDayMax(r1,i);
      }
    }
    for(i=fed1;i<ed2;i++){ 
      if(RoomDayLoad(r2,i)>1 && RoomAgePres(r2,i,ag2)==1){
        if(ag2==RoomDayMin(r2,i))
          delta+=ag2-NewProjMin(r2,i);
        else if(ag2==RoomDayMax(r2,i))
          delta+=NewProjMax(r2,i)-ag2;
      }
    }
  }
  return delta;
}

int IH_Output::OverlappingSwapAgeMix(int p1,int p2) const{
  int delta=0,i;
  int r1=Room(p1),r2=Room(p2),d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  int ag1=in.PatientAgeGroup(p1),ag2=in.PatientAgeGroup(p2);
  bool olp= (d1<d2 && (d2<ed1)) || (d2<d1 && (d1<ed2));
  bool folp=(d1<d2 && (d2<fed2)) || (d2<d1 && (d1<fed1));
  int p_left,p_right,st_left,st_right,d_left,d_right;
  if(r1==r2 && d1==d2)
    return delta;
  if(d1<d2){
    p_left=p1;
    p_right=p2;
    st_left=st1;
    st_right=st2;
    d_left=d1;
    d_right=d2;
    }
  else{
    p_left=p2;
    p_right=p1;
    st_left=st2;
    st_right=st1;
    d_left=d2;
    d_right=d1;
  }
  int left_end  = min(admission_day[p_left]+st_left,in.Days()),   left_fend  = min(admission_day[p_left]+st_right,in.Days());
  int right_end = min(admission_day[p_right]+st_right,in.Days()), right_fend = min(admission_day[p_right]+st_left,in.Days());
  if(olp && !folp){ 
    if(ag1==ag2){
      for(i=left_fend;i<d_right;i++)
        delta+=SubAgeGroup(p_left,i);
      for(i=max(right_end,left_end);i<right_fend;i++)
        delta+=AddAgeGroup(p_right,i);
    }
    else{
      for(i=d_left;i<left_fend;i++)
        delta+=SwapAgeGroups(p_left,p_right,i);
      for(i=left_fend;i<d_right;i++)
        delta+=SubAgeGroup(p_left,i);
      for(i=d_right;i<right_end;i++)
        delta+=SwapAgeGroups(p_right,p_left,i);
      for(i=right_end;i<right_fend;i++)
        delta+=AddAgeGroup(p_left,i);
    }
  }
  else if(olp && folp){
    if(st_left>st_right){ 
      if(ag1==ag2){
        for(i=max(right_end,left_end);i<right_fend;i++)
          delta+=AddAgeGroup(p_left,i);
      }
      else{
        for(i=d_left;i<d_right;i++)
          delta+=SwapAgeGroups(p_left,p_right,i);
        for(i=left_fend;i<min(left_end,right_end);i++)
          delta+=SubAgeGroup(p_right,i);
        for(i=left_end;i<right_end;i++)
          delta+=SwapAgeGroups(p_right,p_left,i);
        for(i=max(right_end,left_end);i<right_fend;i++)
          delta+=AddAgeGroup(p_left,i);
      }
    }
    else{ 
      if(ag1==ag2){
        for(i=max(right_fend,left_fend);i<right_end;i++)
          delta+=SubAgeGroup(p_right,i);
      }
      else{
        for(i=d_left;i<d_right;i++)
          delta+=SwapAgeGroups(p_left,p_right,i);
        for(i=left_end;i<min(left_fend,right_fend);i++)
          delta+=AddAgeGroup(p_left,i);
        for(i=left_fend;i<right_fend;i++)
          delta+=SwapAgeGroups(p_right,p_left,i);
        for(i=max(right_fend,left_fend);i<right_end;i++)
          delta+=SubAgeGroup(p_right,i);
      }
    }
  }
  else{ 
    if(ag1==ag2){
      for(i=left_end;i<d_right;i++)
        delta+=AddAgeGroup(p_right,i);
      for(i=max(right_fend,left_fend);i<right_end;i++)
        delta+=SubAgeGroup(p_right,i);
    }
    else{
      for(i=d_left;i<left_end;i++)
        delta+=SwapAgeGroups(p_left,p_right,i);
      for(i=left_end;i<d_right;i++)
        delta+=AddAgeGroup(p_right,i);
      for(i=d_right;i<min(left_fend,right_fend);i++)
        delta+=AddAgeGroup(p_left,i);
      for(i=left_fend;i<right_fend;i++)
        delta+=SwapAgeGroups(p_right,p_left,i);
      for(i=max(right_fend,left_fend);i<right_end;i++)
        delta+=SubAgeGroup(p_right,i);
    }
  }
  return delta;
}

int IH_Output::SwapSkill(int p1,int p2) const{
  int delta=0,i;
  int d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  for(i=d1;i<min(ed1,fed2);i++)
    delta+=ReplaceSkill(p1,p2,i);
  for(i=d2;i<min(ed2,fed1);i++)
    delta+=ReplaceSkill(p2,p1,i);
  if(st1>st2){ 
    for(i=fed2;i<ed1;i++) 
      delta+=SubSkill(p1,i);
    for(i=ed2;i<fed1;i++)
      delta+=AddSkill(p1,p2,i);
  }
  else if(st1<st2){
    for(i=ed1;i<fed2;i++) 
      delta+=AddSkill(p2,p1,i);
    for(i=fed1;i<ed2;i++)  
      delta+=SubSkill(p2,i);
  }
  return delta;
}

int IH_Output::OverlappingSwapExcess(int p1,int p2) const{
  int delta=0,i,j,n,sh,l_sh,r_sh,diff=0;
  int r1=Room(p1),r2=Room(p2),d1=admission_day[p1],d2=admission_day[p2],st1=in.PatientLengthOfStay(p1),st2=in.PatientLengthOfStay(p2);
  int ed1=d1==-1?-1:min(d1+st1,in.Days()),ed2=d2==-1?-1:min(d2+st2,in.Days());
  int fed2=d1==-1?-1:min(d1+st2,in.Days()),fed1=d2==-1?-1:min(d2+st1,in.Days());
  bool olp= (d1<=d2 && (d2<ed1)) || (d2<=d1 && (d1<ed2));
  bool folp=(d1<=d2 && (d2<fed2)) || (d2<=d1 && (d1<fed1));
  int p_left,p_right,st_left,st_right,d_left,d_right,r_left,r_right;
  if(r1==r2 && d1==d2)
    return delta;
  if(d1<d2){
    p_left=p1;
    p_right=p2;
    st_left=st1;
    st_right=st2;
    d_left=d1;
    d_right=d2;
    r_left=r1;
    r_right=r2;
  }
  else{
    p_left=p2;
    p_right=p1;
    st_left=st2;
    st_right=st1;
    d_left=d2;
    d_right=d1;
    r_left=r2;
    r_right=r1;
  }
  int left_end  = min(admission_day[p_left]+st_left,in.Days()),   left_fend  = min(admission_day[p_left]+st_right,in.Days());
  int right_end = min(admission_day[p_right]+st_right,in.Days()), right_fend = min(admission_day[p_right]+st_left,in.Days());
  vector<int>overlapping_p = FindCurrentOverlappingNurses(p_left,p_right);
  vector<int>overlapping_f = FindFutureOverlappingNurses(p_left,p_right);
  int idx_cur=0,idxl=0,idxr=0,idx_fut=0;
  vector<int>overlappings;
  if(st_left>st_right)
    overlappings = FindOverlappingNurses(p_left,d_right,r_right);
  else
    overlappings = FindOverlappingNurses(p_right,d_left,r_left);
  if(olp && !folp){ 
    for(i=d_left;i<d_right;i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=SwapWorkloads(p_left,p_right,sh);
      }
    for(i=d_right;i<min(left_end,right_end);i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx_cur>=overlapping_p.size() || sh!=overlapping_p[idx_cur]){ 
          delta+=SwapWorkloads(p_left,p_right,sh);
          delta+=SwapWorkloads(p_right,p_left,sh);
        }
        else{ 
          n=NurseAttending(r_left,sh);
          l_sh=sh-(d_left*in.ShiftsPerDay());
          r_sh=sh-(d_right*in.ShiftsPerDay());
          diff=in.PatientWorkloadProduced(p_left,r_sh)-(in.PatientWorkloadProduced(p_left,l_sh)+in.PatientWorkloadProduced(p_right,r_sh));
          delta+=WorkloadDelta(n,sh,diff);
          if(sh==overlappings[idxl])
            idxl++;
          idx_cur++;
        }
      }
    for(i=right_end;i<left_end;i++)
      for(j=0;j<in.ShiftsPerDay();j++){ 
        sh=(i*in.ShiftsPerDay())+j;
        if(idxl>=overlappings.size() || sh!=overlappings[idxl]){
          delta+=RemoveWorkload(p_left,sh);
          delta+=AddWorkload(p_left,p_right,sh);
        }
        else{
          n=NurseAttending(r_right,sh);
          l_sh=sh-(d_left*in.ShiftsPerDay());
          r_sh=sh-(d_right*in.ShiftsPerDay());
          diff=in.PatientWorkloadProduced(p_left,r_sh)-in.PatientWorkloadProduced(p_left,l_sh);
          delta+=WorkloadDelta(n,sh,diff);
          idxl++;
        }
      }
    for(i=left_end;i<right_end;i++)
      for(j=0;j<in.ShiftsPerDay();j++){
      sh=(i*in.ShiftsPerDay())+j;
      delta+=SwapWorkloads(p_right,p_left,sh);
    }
    for(i=max(right_end,left_end);i<right_fend;i++)
      for(j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=AddWorkload(p_left,p_right,sh);
      }
  }
  else if(olp && folp){
    if(st_left>st_right){ 
      for(i=d_left;i<d_right;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=SwapWorkloads(p_left,p_right,sh);
      }
      for(i=d_right;i<left_fend;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idx_fut>=overlapping_f.size() || sh!=overlapping_f[idx_fut]){ 
           delta+=SwapWorkloads(p_left,p_right,sh);
           delta+=SwapWorkloads(p_right,p_left,sh);
          }
          else{
            n=NurseAttending(r_left,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_left,r_sh)+in.PatientWorkloadProduced(p_right,l_sh)-(in.PatientWorkloadProduced(p_left,l_sh)+in.PatientWorkloadProduced(p_right,r_sh));
            delta+=WorkloadDelta(n,sh,diff);
            idx_fut++;
            idx_cur++;
            if(sh==overlappings[idxl])
              idxl++;
          }
      }
      for(i=left_fend;i<min(left_end,right_end);i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idx_cur>=overlapping_p.size() || sh!=overlapping_p[idx_cur]){
            delta+=SwapWorkloads(p_left,p_right,sh);
            delta+=SwapWorkloads(p_right,p_left,sh);
          }
          else{
            n=NurseAttending(r_left,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_left,r_sh)-(in.PatientWorkloadProduced(p_left,l_sh)+in.PatientWorkloadProduced(p_right,r_sh));
            delta+=WorkloadDelta(n,sh,diff);
            if(sh==overlappings[idxl])
              idxl++;
            idx_cur++;
          }
      }
      for(i=right_end;i<left_end;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idxl>=overlappings.size() || sh!=overlappings[idxl]){
            delta+=RemoveWorkload(p_left,sh);
            delta+=AddWorkload(p_left,p_right,sh);
          }
          else{
            n=NurseAttending(r_left,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_left,r_sh)-(in.PatientWorkloadProduced(p_left,l_sh));
            delta+=WorkloadDelta(n,sh,diff);
            idxl++;
          }
        }
      for(i=left_end;i<right_end;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          delta+=SwapWorkloads(p_right,p_left,sh);
        }
      for(i=max(right_end,left_end);i<right_fend;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          delta+=AddWorkload(p_left,p_right,sh);
       }
    }  
    else{ 
      for(i=d_left;i<d_right;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=SwapWorkloads(p_left,p_right,sh);
      }
      for(i=d_right;i<left_end;i++)
        for (j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idx_cur>=overlapping_p.size() || sh!=overlapping_p[idx_cur]){
            delta+=SwapWorkloads(p_left,p_right,sh);
            delta+=SwapWorkloads(p_right,p_left,sh);
          }
          else{
            n=NurseAttending(r_left,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_left,r_sh)+in.PatientWorkloadProduced(p_right,l_sh)-(in.PatientWorkloadProduced(p_left,l_sh)+in.PatientWorkloadProduced(p_right,r_sh));
            delta+=WorkloadDelta(n,sh,diff);
            idx_cur++;
            idx_fut++;
            if(sh==overlappings[idxr])
              idxr++;
          }
        }
      for(i=left_end;i<min(left_fend,right_fend);i++)
        for(j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idx_fut>=overlapping_f.size() || sh!=overlapping_f[idx_fut]){
            delta+=SwapWorkloads(p_left,p_right,sh);
            delta+=SwapWorkloads(p_right,p_left,sh);
          }
          else{
            n=NurseAttending(r_left,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_left,r_sh)+in.PatientWorkloadProduced(p_right,l_sh)-in.PatientWorkloadProduced(p_right,r_sh);
            delta+=WorkloadDelta(n,sh,diff);
            idx_fut++;
            if(sh==overlappings[idxr])
              idxr++;
          }
        }
      for(i=right_fend;i<left_fend;i++)
        for(j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          if(idxr>=overlappings.size() || sh!=overlappings[idxr]){
            delta+=RemoveWorkload(p_right,sh);
            delta+=AddWorkload(p_right,p_left,sh);
          }
          else{
            n=NurseAttending(r_right,sh);
            l_sh=sh-(d_left*in.ShiftsPerDay());
            r_sh=sh-(d_right*in.ShiftsPerDay());
            diff=in.PatientWorkloadProduced(p_right,l_sh)-in.PatientWorkloadProduced(p_right,r_sh);
            delta+=WorkloadDelta(n,sh,diff);
            idxr++;
          }
        }
      for(i=left_fend;i<right_fend;i++)
        for(j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          delta+=SwapWorkloads(p_right,p_left,sh);
        }
      for(i=max(right_fend,left_fend);i<right_end;i++)
        for(j=0;j<in.ShiftsPerDay();j++){
          sh=(i*in.ShiftsPerDay())+j;
          delta+=RemoveWorkload(p_right,sh);
        }
    }
  }
  else{ 
    for(i=d_left;i<d_right;i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=SwapWorkloads(p_left,p_right,sh);
      }
    for(i=d_right;i<min(left_fend,right_fend);i++) 
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idx_fut>=overlapping_f.size() || sh!=overlapping_f[idx_fut]){ 
          delta+=SwapWorkloads(p_left,p_right,sh);
          delta+=SwapWorkloads(p_right,p_left,sh);
        }
        else{ 
          n=NurseAttending(r_left,sh);
          l_sh=sh-(d_left*in.ShiftsPerDay());
          r_sh=sh-(d_right*in.ShiftsPerDay());
          diff=in.PatientWorkloadProduced(p_left,r_sh)+in.PatientWorkloadProduced(p_right,l_sh)-in.PatientWorkloadProduced(p_right,r_sh);
          delta+=WorkloadDelta(n,sh,diff);
          if(sh==overlappings[idxr])
            idxr++;
          idx_fut++;
        }
      }
    for(i=right_fend;i<left_fend;i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        if(idxr>=overlappings.size() || sh!=overlappings[idxr]){ 
          delta+=AddWorkload(p_right,p_left,sh);
          delta+=RemoveWorkload(p_right,sh);
        }
        else{
          n=NurseAttending(r_right,sh);
          l_sh=sh-(d_left*in.ShiftsPerDay());
          r_sh=sh-(d_right*in.ShiftsPerDay());
          diff=in.PatientWorkloadProduced(p_right,l_sh)-in.PatientWorkloadProduced(p_right,r_sh);
          delta+=WorkloadDelta(n,sh,diff);
          idxr++;
        }
      }
    for(i=left_fend;i<right_fend;i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=SwapWorkloads(p_right,p_left,sh);
      }
    for(i=max(right_fend,left_fend);i<right_end;i++)
      for (j=0;j<in.ShiftsPerDay();j++){
        sh=(i*in.ShiftsPerDay())+j;
        delta+=RemoveWorkload(p_right,sh);
      }
  }
  return delta;
}

int IH_Output::SwapGenders(int p,int d) const{
  int delta=0,r=room[p];
  Gender g=in.PatientGender(p);
  if(g==Gender::A){
   if(RoomDayAPatients(r,d)<=RoomDayBPatients(r,d))
     delta--;
   else if(RoomDayBPatients(r,d)==0 && RoomDayAPatients(r,d)>1)
     delta++;
   else if(RoomDayAPatients(r,d)==1 && RoomDayBPatients(r,d)>0)
     delta--;
   else if(RoomDayBPatients(r,d)+1>=RoomDayAPatients(r,d)-1)
     delta+=(RoomDayAPatients(r,d)-1)-RoomDayBPatients(r,d);
   else
     delta++;
  }
  else{
    if(RoomDayBPatients(r,d)<=RoomDayAPatients(r,d))
      delta--;
    else if(RoomDayAPatients(r,d)==0 && RoomDayBPatients(r,d)>1)
      delta++;
    else if(RoomDayBPatients(r,d)==1 && RoomDayAPatients(r,d)>0)
      delta--;
    else if(RoomDayAPatients(r,d)+1>=RoomDayBPatients(r,d)-1)
      delta+=(RoomDayBPatients(r,d)-1)-RoomDayAPatients(r,d);
    else
      delta++;
  }
  return delta;
}

int IH_Output::SubGender(int p,int d) const{    
  int delta=0,r=room[p];
  Gender g1=in.PatientGender(p);
  if(g1==Gender::A){
     if(RoomDayAPatients(r,d)<=RoomDayBPatients(r,d))
       delta--;
  }
  else{
    if(RoomDayBPatients(r,d)<=RoomDayAPatients(r,d))
      delta--;
  }
  return delta;
}

int IH_Output::AddGender(int p,int d) const{     
  int delta=0,r=room[p];
  Gender g1=in.PatientGender(p);
  if(g1==Gender::A){
     if(RoomDayAPatients(r,d)<RoomDayBPatients(r,d))
       delta++;
  }
  else{
    if(RoomDayBPatients(r,d)<RoomDayAPatients(r,d))
      delta++;
  }
  return delta;
}

int IH_Output::AddAgeGroup(int p,int d) const{
  int delta=0,r=room[p];
  int ag=in.PatientAgeGroup(p); 
  if(RoomDayLoad(r,d)>0){
    if(ag<RoomDayMin(r,d))
      delta+=RoomDayMin(r,d)-ag;
    else if(ag>RoomDayMax(r,d))
      delta+=ag-RoomDayMax(r,d);
  }
  return delta;
}

int IH_Output::SubAgeGroup(int p,int d) const{
  int delta=0,r=room[p];
  int ag=in.PatientAgeGroup(p);
  if(RoomDayLoad(r,d)>1 && RoomAgePres(r,d,ag)==1){
    if(ag==RoomDayMin(r,d))
      delta+=ag-NewProjMin(r,d);
    if(ag==RoomDayMax(r,d))
      delta+=NewProjMax(r,d)-ag;
  }
  return delta;
}

int IH_Output::SwapAgeGroups(int p1,int p2,int d) const{
  int delta=0,r=room[p1];
  int ag1=in.PatientAgeGroup(p1),ag2=in.PatientAgeGroup(p2),candidate;
  if(RoomDayLoad(r,d)>1){ 
    if(RoomAgePres(r,d,ag1)>1 && ag1==RoomDayMin(r,d) && ag1==RoomDayMax(r,d)) 
        delta+=abs(ag2-ag1);
      else{
        if(ag1==RoomDayMin(r,d)){ 
          if(ag2<ag1)
            delta+=ag1-ag2;
          else if(RoomAgePres(r,d,ag1)==1){ 
            candidate=NewProjMin(r,d);
            delta+=(candidate>ag2)? ag1-ag2:ag1-candidate;
          }
          if(ag2>RoomDayMax(r,d))
            delta+=ag2-RoomDayMax(r,d);
        }
        if(ag1==RoomDayMax(r,d)){ 
          if(ag2>ag1)
            delta+=ag2-ag1;
          else if(RoomAgePres(r,d,ag1)==1){ 
            candidate=NewProjMax(r,d);
            delta+=(candidate<ag2)? ag2-ag1:candidate-ag1;
          }
          if(ag2<RoomDayMin(r,d))
            delta+=RoomDayMin(r,d)-ag2;
        }
        else if(ag1!=RoomDayMax(r,d) && ag1!=RoomDayMin(r,d)){ 
          if(ag2<RoomDayMin(r,d))
            delta+=RoomDayMin(r,d)-ag2;
          else if(ag2>RoomDayMax(r,d))
            delta+=ag2-RoomDayMax(r,d);
        }
      }
  }
  return delta;
}

int IH_Output::ReplaceSkill(int p1,int p2,int d) const{ 
  int delta=0,i,sh,rel_sh,skn,sk1,sk2;
  int r=room[p1],ds=admission_day[p1];
  for(i=0;i<in.ShiftsPerDay();i++){
    sh=(d*in.ShiftsPerDay())+i;
    rel_sh=sh-(ds*in.ShiftsPerDay());
    sk1=in.PatientSkillLevelRequired(p1,rel_sh);
    sk2=in.PatientSkillLevelRequired(p2,rel_sh);
    skn=in.NurseSkillLevel(NurseAttending(r,sh));
    if(sk1!=sk2){ 
      if(sk1>skn)
        delta+=skn-sk1;
      if(sk2>skn)
        delta+=sk2-skn;
    }
  }
  return delta;
}

int IH_Output::AddSkill(int p,int p2,int d) const{
  int delta=0,ds=admission_day[p2],r=room[p2],i;
  int sh,rel_sh,skn,sk;
  for(i=0;i<in.ShiftsPerDay();i++){
    sh=(d*in.ShiftsPerDay())+i;
    rel_sh=sh-(ds*in.ShiftsPerDay());
    sk=in.PatientSkillLevelRequired(p,rel_sh);
    skn=in.NurseSkillLevel(NurseAttending(r,sh));
    if(sk>skn)
      delta+=sk-skn;
  }
  return delta;
}

int IH_Output::SubSkill(int p,int d) const{
  int delta=0,r=room[p],ds=admission_day[p] ,i;
  int sh,rel_sh,skn,sk;
  for(i=0;i<in.ShiftsPerDay();i++){
    sh=(d*in.ShiftsPerDay())+i;
    rel_sh=sh-(ds*in.ShiftsPerDay());
    sk=in.PatientSkillLevelRequired(p,rel_sh);
    skn=in.NurseSkillLevel(NurseAttending(r,sh));
    if(sk>skn)
      delta+=skn-sk;
  }
  return delta;
}

int IH_Output::AddWorkload(int p1,int p2,int sh) const{
  int delta=0;
  int rel_sh,n,nwl;
  int d=admission_day[p2],r=room[p2];
  rel_sh=sh-(d*in.ShiftsPerDay());
  nwl=in.PatientWorkloadProduced(p1,rel_sh);
  n=NurseAttending(r,sh);
  if(NurseShiftLoad(n,sh)>in.NurseMaxLoad(n,sh)){ 
    if(NurseShiftLoad(n,sh)+nwl>in.NurseMaxLoad(n,sh))
      delta+=nwl;
    else
      delta-=NurseShiftLoad(n,sh)-in.NurseMaxLoad(n,sh);
  } 
  else if(NurseShiftLoad(n,sh)+nwl>in.NurseMaxLoad(n,sh))
    delta+=NurseShiftLoad(n,sh)+nwl-in.NurseMaxLoad(n,sh);
  return delta;
}

int IH_Output::RemoveWorkload(int p,int sh) const{
  int delta=0,rel_sh,n,owl;
  int d=admission_day[p],r=room[p];
  rel_sh=sh-(d*in.ShiftsPerDay());
  n=NurseAttending(r,sh);
  owl=in.PatientWorkloadProduced(p,rel_sh);
  if(NurseShiftLoad(n,sh)>in.NurseMaxLoad(n,sh)){ 
    if(NurseShiftLoad(n,sh)-owl>in.NurseMaxLoad(n,sh))
      delta-=owl;
    else
      delta-=NurseShiftLoad(n,sh)-in.NurseMaxLoad(n,sh);
  } 
  return delta;
}


int IH_Output::SwapWorkloads(int p1,int p2, int sh) const{ 
  int delta=0;
  int rel_sh,n,wl1,wl2;
  int d1=admission_day[p1],r1=room[p1];
  rel_sh=sh-(d1*in.ShiftsPerDay());
  n=NurseAttending(r1,sh);
  if(rel_sh<in.PatientLengthOfStay(p1)*in.ShiftsPerDay())
    wl1=in.PatientWorkloadProduced(p1,rel_sh);
  else
    wl1=0;
  if(rel_sh<in.PatientLengthOfStay(p2)*in.ShiftsPerDay())
    wl2=in.PatientWorkloadProduced(p2,rel_sh);
  else
    wl2=0;
  if(wl1!=wl2){
    if(NurseShiftLoad(n,sh)>in.NurseMaxLoad(n,sh)){ 
      if(NurseShiftLoad(n,sh)+(wl2-wl1)>in.NurseMaxLoad(n,sh))
        delta+=wl2-wl1;
      else
        delta-=NurseShiftLoad(n,sh)-in.NurseMaxLoad(n,sh);
    } 
    else if(NurseShiftLoad(n,sh)+(wl2-wl1)>in.NurseMaxLoad(n,sh))
      delta+=NurseShiftLoad(n,sh)+(wl2-wl1)-in.NurseMaxLoad(n,sh);
  }
  return delta;
}

int IH_Output::WorkloadDelta(int n,int sh,int d) const{
  int delta=0;
  if(d==0)
    return delta;
  if(NurseShiftLoad(n,sh)>in.NurseMaxLoad(n,sh)){ 
   if(NurseShiftLoad(n,sh)+d>in.NurseMaxLoad(n,sh)) 
     delta+=d;
   else
     delta-=NurseShiftLoad(n,sh)-in.NurseMaxLoad(n,sh); 
   } 
  else if(NurseShiftLoad(n,sh)+d>in.NurseMaxLoad(n,sh))
    delta+=NurseShiftLoad(n,sh)+d-in.NurseMaxLoad(n,sh);
  return delta;
}

bool IH_Output::Admissable(int p,int d) const{
  if(d==-1){
    if(in.PatientMandatory(p))
      return false;
  }
  else{ 
    if(!in.PatientAdmissable(p,d))
      return false;
  }
  return true;
}
