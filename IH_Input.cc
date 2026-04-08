// File IH_Input.cc
#include <sstream>
#include <fstream>
#include "json.hpp" 
#include "IH_Input.hh"

ostream& operator<<(ostream& os, const Occupant& oc){
  os << oc.id << ' ' <<oc.assigned_room << ' ' 
     << (oc.gender == Gender::A ? 'A' : 'B') << ' ' 
     << oc.age_group << " " << oc.length_of_stay << " ";
  for (int s = 0; s < oc.workload_produced.size(); s++)
    os << oc.workload_produced[s] << '/' << oc.skill_level_required[s] << (s < oc.workload_produced.size() - 1 ? ',' : ']');
  return os;
}

ostream& operator<<(ostream& os, const Patient& pat)
{
  os << pat.id << (pat.mandatory ? '*' : ' ') << ' ' 
     << (pat.gender == Gender::A ? 'A' : 'B') << ' ' 
     << pat.age_group << " " << pat.length_of_stay << " " 
     << pat.surgery_release_day << " " << pat.surgery_due_day << " "
     << pat.surgery_duration << " " << pat.surgeon << " [";
  for (int s = 0; s < pat.workload_produced.size(); s++)
    os << pat.workload_produced[s] << '/' << pat.skill_level_required[s] << (s < pat.workload_produced.size() - 1 ? ',' : ']');
  os << " (";
  for (int r = 0; r < pat.incompatible_rooms.size(); r++)
    if (pat.incompatible_rooms[r])
      os << r << " ";
  os << ")";
  return os;
}

ostream& operator<<(ostream& os, const Surgeon& sur)
{
  os << sur.id << " [";
  for (int s = 0; s < sur.max_surgery_time.size(); s++)
    os << sur.max_surgery_time[s] << (s < sur.max_surgery_time.size() - 1 ? ',' : ']');
  return os;
}

ostream& operator<<(ostream& os, const OperatingTheater& op_room)
{
  os << op_room.id << " [";
  for (int ot = 0; ot < op_room.availability.size(); ot++)
    os << op_room.availability[ot] << (ot < op_room.availability.size() - 1 ? ',' : ']');
  return os;
}

ostream& operator<<(ostream& os, const Room& room)
{
  os << room.id << " (" << room.capacity << ")";
  return os;
}

ostream& operator<<(ostream& os, const Nurse& nur)
{
  int i, s;
  os << nur.id << " " << nur.skill_level << " {";
  for (i = 0; i < nur.working_shifts.size(); i++)
    os << nur.working_shifts[i] << (i < nur.working_shifts.size() - 1 ? ',' : '}');
  os << " [";
  for (s = 0; s < nur.is_working_in_shift.size(); s++)
    os << nur.is_working_in_shift[s] << '/' << nur.max_loads[s] << (s < nur.is_working_in_shift.size() - 1  ? ',' : ']');
  return os;
}

ostream& operator<<(ostream& os, const IH_Input& in)
{
  int p, r, s, ot, n, i;
  os << "Patients:" << endl;
  for (p = 0; p < in.patients; p++)
    os << in.patients_vect[p] << endl;
  os << "Surgeons:" << endl;
  for (s = 0; s < in.surgeons; s++)
    os << in.surgeons_vect[s] << endl;
  os << "Operating rooms:" << endl;
  for (ot = 0; ot < in.operating_theaters; ot++)
    os << in.operating_theaters_vect[ot] << endl;
  os << "Rooms:" << endl;
  for (r = 0; r < in.rooms; r++)
    os << in.rooms_vect[r] << endl;
  os << "Nurses:" << endl;
  for (n = 0; n < in.nurses; n++)
    os << in.nurses_vect[n] << endl;

  os << "Available nurses per shift:" << endl;
  for (s = 0; s < in.shifts; s++)
  {
     os << s << ": ";
     for (i = 0; i < in.available_nurses[s].size(); i++)
        os << in.available_nurses[s][i] << (i < in.available_nurses[s].size() - 1 ? ", ": "\n");
  }
  return os;
}

IH_Input::IH_Input(string file_name)
{
  int i, d, p, n, f, ag, sn, s, r, ot, ed, cd;
  optional=0;
  nlohmann::json j_in, j_p, j_r, j_s, j_ot, j_n, j_f;
  ifstream is(file_name);
  if(!is)
  {
    throw runtime_error("Cannot open input file " + file_name);
  }
  is >> j_in;
  days = j_in["days"];
  vector<int> total_ot_availability(days,0);
  shifts_per_day = j_in["shift_types"].size();
  shifts = days * shifts_per_day;
  skill_levels = j_in["skill_levels"];
  patients = j_in["patients"].size();
  occupants = j_in["occupants"].size();
  surgeons = j_in["surgeons"].size();
  operating_theaters = j_in["operating_theaters"].size();
  rooms = j_in["rooms"].size();
  nurses = j_in["nurses"].size();
  
  age_groups = j_in["age_groups"].size();
  ResizeDataStructures();

  nlohmann::json jweights = j_in["weights"];
  weights.resize(jweights.size());
  weights[0] = jweights["unscheduled_optional"];
  weights[1] = jweights["patient_delay"];
  weights[2] = jweights["open_operating_theater"];
  weights[3] = jweights["room_mixed_age"];
  weights[4] = jweights["room_nurse_skill"];
  weights[5] = jweights["nurse_eccessive_workload"];
  weights[6] = jweights["continuity_of_care"];
  weights[7] = jweights["surgeon_transfer"];
  
  for (sn = 0; sn < shifts_per_day; sn++)
    shift_names[sn] =  j_in["shift_types"][sn];
  for (ag = 0; ag < age_groups; ag++)
    age_group_names[ag] = j_in["age_groups"][ag];
  for (ot = 0; ot < operating_theaters; ot++)
  {
    j_ot = j_in["operating_theaters"][ot];
    operating_theaters_vect[ot].id = j_ot["id"];
    for (d = 0; d < days; d++){
      operating_theaters_vect[ot].availability[d] = j_ot["availability"][d];
      total_ot_availability[d]+=int(j_ot["availability"][d]);
    }
  }
  for (s = 0; s < surgeons; s++)
  {
    j_s = j_in["surgeons"][s];
    surgeons_vect[s].id = j_s["id"];
    for (d = 0; d < days; d++){
      if(j_s["max_surgery_time"][d]<total_ot_availability[d])
        surgeons_vect[s].max_surgery_time[d] = j_s["max_surgery_time"][d];
      else
        surgeons_vect[s].max_surgery_time[d] = total_ot_availability[d];
    }
  }
  for (r = 0; r < rooms; r++)
  { 
    j_r = j_in["rooms"][r];
    rooms_vect[r].id = j_r["id"];
    rooms_vect[r].capacity = j_r["capacity"];
  }
  for (f=0;f<occupants;f++){
    j_f=j_in["occupants"][f];
    occupants_vect[f].id=j_f["id"];
    if (j_f["gender"] == "A"){
      occupants_vect[f].gender = Gender::A;
    }
    else
      occupants_vect[f].gender = Gender::B;
    occupants_vect[f].age_group = FindAgeGroup(j_f["age_group"]);
    occupants_vect[f].length_of_stay = j_f["length_of_stay"];
    occupants_vect[f].workload_produced.resize(occupants_vect[f].length_of_stay * shifts_per_day);
    occupants_vect[f].skill_level_required.resize(occupants_vect[f].length_of_stay * shifts_per_day);
    for (s = 0; s < occupants_vect[f].length_of_stay * shifts_per_day; s++)
    {
      occupants_vect[f].workload_produced[s] = j_f["workload_produced"][s];
      occupants_vect[f].skill_level_required[s] = j_f["skill_level_required"][s];
    } 
    r=FindRoom(j_f["room_id"]);
    occupants_vect[f].assigned_room=r;
    for(i=0;i<occupants_vect[f].length_of_stay;i++)
      room_day_fixed_list[r][i].emplace_back(f);
  }
  for (p = 0; p < patients; p++)
  {
    j_p = j_in["patients"][p];
    patients_vect[p].id = j_p["id"];
    patients_vect[p].mandatory = j_p["mandatory"];
    if (j_p["gender"] == "A")
      patients_vect[p].gender = Gender::A;
    else
      patients_vect[p].gender = Gender::B;
    patients_vect[p].age_group = FindAgeGroup(j_p["age_group"]);
    patients_vect[p].length_of_stay = j_p["length_of_stay"];
    patients_vect[p].surgery_release_day = j_p["surgery_release_day"];
    if (patients_vect[p].mandatory)
      patients_vect[p].surgery_due_day = j_p["surgery_due_day"];
    else{
      patients_vect[p].surgery_due_day = -1;
      optional++;
    }
    
    ed=patients_vect[p].mandatory?(patients_vect[p].surgery_due_day+1)-patients_vect[p].surgery_release_day:days-patients_vect[p].surgery_release_day;
    patients_vect[p].surgery_duration = j_p["surgery_duration"];
    patients_vect[p].surgeon = FindSurgeon(j_p["surgeon_id"]);
    for(i=0;i<ed;i++){
      cd=patients_vect[p].surgery_release_day+i;
      if (surgeons_vect[patients_vect[p].surgeon].max_surgery_time[cd]>patients_vect[p].surgery_duration){
        if (total_ot_availability[cd]>= patients_vect[p].surgery_duration && (patients_vect[p].mandatory || weights[0]>(weights[6]*shifts_per_day)+(i*weights[1]))){
          patients_vect[p].incompatible_days[cd]=false;
          patients_vect[p].possible_admissions.emplace_back(cd);
        }
      }
    }
    if (!j_p["incompatible_room_ids"].is_null())
    {
      for (i = 0; i < j_p["incompatible_room_ids"].size(); i++)
      {
        r = FindRoom(j_p["incompatible_room_ids"][i]);
        patients_vect[p].incompatible_rooms[r] = true;
      }
    }
    patients_vect[p].workload_produced.resize(patients_vect[p].length_of_stay * shifts_per_day);
    patients_vect[p].skill_level_required.resize(patients_vect[p].length_of_stay * shifts_per_day);
    for (s = 0; s < patients_vect[p].length_of_stay * shifts_per_day; s++)
    {
      patients_vect[p].workload_produced[s] = j_p["workload_produced"][s];
      patients_vect[p].skill_level_required[s] = j_p["skill_level_required"][s];
    }
    
    for(int p2=0;p2<p;p2++)
      for(d=0;d<days;d++){
        if(!patients_vect[p].incompatible_days[d] && !patients_vect[p2].incompatible_days[d]){
          swappable[p][d].emplace_back(p2);
          swappable[p2][d].emplace_back(p);
        }
      }
  }
  for (i = 0; i < patients; i++){
    if(patients_vect[i].possible_admissions.size()==0)
      throw runtime_error("Patient " + patients_vect[i].id + " cannot be admitted in any day");
  }
  for (n = 0; n < nurses; n++)
  {
    j_n = j_in["nurses"][n];
    nurses_vect[n].id = j_n["id"];
    nurses_vect[n].skill_level = j_n["skill_level"];
    for (i = 0; i <  j_n["working_shifts"].size(); i++)
    { 
      s = static_cast<int>(j_n["working_shifts"][i]["day"]) * shifts_per_day + FindShift(j_n["working_shifts"][i]["shift"]);
      nurses_vect[n].working_shifts.emplace_back(s);
      nurses_vect[n].is_working_in_shift[s] = true;
      nurses_vect[n].max_loads[s] = j_n["working_shifts"][i]["max_load"];
      available_nurses[s].emplace_back(n);
    }
  }
}

void IH_Input::ResizeDataStructures()
{
  int p, n, s, ot;
  occupants_vect.resize(occupants);
  patients_vect.resize(patients);
  swappable.resize(patients,vector<vector<int>>(days));
  
  for (p = 0; p < patients; p++){
    patients_vect[p].incompatible_rooms.resize(rooms,false);
    patients_vect[p].incompatible_days.resize(days,true);
  }
  surgeons_vect.resize(surgeons);
  for (s = 0; s < surgeons; s++)
    surgeons_vect[s].max_surgery_time.resize(days);

  operating_theaters_vect.resize(operating_theaters);
  for (ot = 0; ot < operating_theaters; ot++)
    operating_theaters_vect[ot].availability.resize(days);
  rooms_vect.resize(rooms);
  nurses_vect.resize(nurses);  
  for (n = 0; n < nurses; n++)
  {
    nurses_vect[n].is_working_in_shift.resize(shifts, false);
    nurses_vect[n].max_loads.resize(shifts, 0);
  }
  available_nurses.resize(shifts);
  shift_names.resize(shifts_per_day);
  age_group_names.resize(age_groups);
  room_day_fixed_list.resize(rooms,vector<vector<int>>(days));
 
}

int IH_Input::FindAgeGroup(string age_group_name) const
{
  for (int i = 0; i < age_groups; i++)
    if (age_group_name == age_group_names[i])
      return i;
  throw invalid_argument("Unknown age group name");       
}

int IH_Input::FindSurgeon(string surgeon_id) const
{
  for (int i = 0; i < surgeons; i++)
    if (surgeon_id == surgeons_vect[i].id)
      return i;
  throw invalid_argument("Unknown surgeon id");        
}

int IH_Input::FindShift(string shift_name) const
{
  for (int i = 0; i < shifts_per_day; i++)
    if (shift_name == shift_names[i])
      return i;
  throw invalid_argument("Unknown shift name");     
}

int IH_Input::FindOT(string ot_name) const
{
  for (int i = 0; i < operating_theaters; i++)
    if (ot_name == operating_theaters_vect[i].id)
      return i;
  throw invalid_argument("Unknown OT id " + ot_name);           
}

int IH_Input::FindRoom(string room_name) const
{
  for (int i = 0; i < rooms; i++)
    if (room_name == rooms_vect[i].id)
      return i;
  throw invalid_argument("Unknown room id " + room_name);     
}

int IH_Input::FindPatient(string patient_id) const
{
  for (int i = 0; i < patients; i++)
    if (patient_id == patients_vect[i].id)
      return i;
  throw invalid_argument("Unknown patient id " + patient_id);           
}

int IH_Input::FindNurse(string nurse_id) const
{
  for (int i = 0; i < nurses; i++)
    if (nurse_id == nurses_vect[i].id)
      return i;
  throw invalid_argument("Unknown nurse id " + nurse_id);     
}

string IH_Input::ShiftDescription(int s) const
{
  stringstream ss;
  ss << s << " (day" << s/shifts_per_day << "@" << shift_names[s%shifts_per_day] << ")";
  return ss.str();
}

bool IH_Input::Overlap(int p,int d1,int d2) const{
  if(d1==-1 || d2==-1)
    return false;
  if (d1<d2 && (d1+patients_vect[p].length_of_stay-1)>=d2)
    return true;
  if (d2<d1 && (d2+patients_vect[p].length_of_stay-1)>=d1) 
    return true;
  return false;
}
