// File IH_Input.hh
#ifndef IH_INPUT_HH
#define IH_INPUT_HH

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

enum class Gender { A, B };

class Occupant
{
  friend ostream& operator<<(ostream& os, const Occupant& pat);
 public: 
  string id;  
  Gender gender;
  int age_group;
  int length_of_stay;
  vector<int> workload_produced; 
  vector<int> skill_level_required;
  int assigned_room;
};

class Patient: public Occupant
{
  friend ostream& operator<<(ostream& os, const Patient& pat);
 public: 
  
  bool mandatory;
  int surgery_release_day;
  int surgery_due_day;
  int surgery_duration;
  int surgeon;
  vector<bool> incompatible_rooms; 
  vector<bool> incompatible_days; 
  vector<int> possible_admissions;
};

class Surgeon
{
  friend ostream& operator<<(ostream& os, const Surgeon& sur);
 public: 
  string id;
  vector<int> max_surgery_time;
};

class OperatingTheater
{
  friend ostream& operator<<(ostream& os, const OperatingTheater& op_room);
 public: 
  string id;
  vector<int> availability;
};

class Room
{
  friend ostream& operator<<(ostream& os, const Room& room);
 public: 
  string id;
  int capacity;
};

class Nurse
{
  friend ostream& operator<<(ostream& os, const Nurse& nur);
 public: 
  string id;
  int skill_level;
  vector<int> working_shifts; 
  vector<bool> is_working_in_shift;
  vector<int> max_loads; 
};

class IH_Input
{
  friend ostream& operator<<(ostream& os, const IH_Input& in);
 public:
  IH_Input(string file_name);
  int Days() const { return days; }
  int ShiftsPerDay() const { return shifts_per_day; }
  int Shifts() const { return shifts; }
  int Occupants() const { return occupants; }
  int Patients() const { return patients; }
  int OptionalPatients() const { return optional; }
  int AgeGroups() const { return age_groups; }
  int Surgeons() const { return surgeons; }
  int OperatingTheaters() const { return operating_theaters; }
  int Rooms() const { return rooms; }
  int Nurses() const { return nurses; }

// Nurses
  string NurseId(int n) const { return nurses_vect[n].id; }
  int NurseSkillLevel(int n) const { return nurses_vect[n].skill_level; }
  bool IsNurseWorkingInShift(int n, int s) const { return nurses_vect[n].is_working_in_shift[s]; }
  int NurseWorkingShifts(int n) const { return nurses_vect[n].working_shifts.size(); }
  int NurseWorkingShift(int n, int i) const { return nurses_vect[n].working_shifts[i]; }
  int AvailableNurses(int s) const { return available_nurses[s].size(); }
  int AvailableNurse(int s, int i) const { return available_nurses[s][i]; }
  int NurseMaxLoad(int n, int s) const { return nurses_vect[n].max_loads[s]; }

// Patients
  string PatientId(int p) const { return patients_vect[p].id; }
  Gender PatientGender(int p) const { return patients_vect[p].gender; }
  int PatientSurgeryReleaseDay(int p) const { return patients_vect[p].surgery_release_day; }
  int PatientAgeGroup(int p) const { return patients_vect[p].age_group; }
  int PatientLengthOfStay(int p) const { return patients_vect[p].length_of_stay; }
  int PatientLastPossibleDay(int p) const { return PatientMandatory(p) ? patients_vect[p].surgery_due_day : days-1; }
  int PatientSurgeryDuration(int p) const { return patients_vect[p].surgery_duration; }
  int PatientSurgeon(int p) const { return patients_vect[p].surgeon; }
  bool PatientMandatory(int p) const { return patients_vect[p].mandatory; }
  bool IncompatibleRoom(int p, int r) const { return patients_vect[p].incompatible_rooms[r]; }
  int PatientSkillLevelRequired (int p, int s) const  { return patients_vect[p].skill_level_required[s]; }
  int PatientWorkloadProduced(int p, int s) const { return patients_vect[p].workload_produced[s]; }
  bool PatientAdmissable(int p,int d) const {return !patients_vect[p].incompatible_days[d];}
  int PatientNumPossibleAdmission(int p) const { return patients_vect[p].possible_admissions.size();}
  int PatientPossibleAdmission(int p,int i) const { return patients_vect[p].possible_admissions[i];}

//Occupants

  string OccupantId(int p) const { return occupants_vect[p].id; }
  Gender OccupantGender(int p) const { return occupants_vect[p].gender; }
  int OccupantAgeGroup(int p) const { return occupants_vect[p].age_group; }
  int OccupantLengthOfStay(int p) const { return occupants_vect[p].length_of_stay; }
  int OccupantRoom(int p) const {return occupants_vect[p].assigned_room; }
  int OccupantSkillLevelRequired(int p, int s) const { return occupants_vect[p].skill_level_required[s]; }
  int OccupantWorkloadProduced(int p, int s) const { return occupants_vect[p].workload_produced[s]; }
  int OccupantsPresent(int r,int d) const { return room_day_fixed_list[r][d].size(); }
  int OccupantPresence(int r,int d,int i) const { return room_day_fixed_list[r][d][i]; }
  int PossibleSwaps(int p,int d) const { return swappable[p][d].size(); }
  int SwappablePatient(int p,int d,int i) const { return swappable[p][d][i]; }

// Rooms
  string RoomId(int r) const { return rooms_vect[r].id; }
  int RoomCapacity(int r) const { return rooms_vect[r].capacity; }

// Operating theaters and surgeons
  string OperatingTheaterId (int t) const { return operating_theaters_vect[t].id; }
  string SurgeonId (int u) const { return surgeons_vect[u].id; }
  int OperatingTheaterAvailability (int t, int d) const { return operating_theaters_vect[t].availability[d]; }
  int SurgeonMaxSurgeryTime (int s, int d) const { return surgeons_vect[s].max_surgery_time[d]; }

  string ShiftName(int s) const { return shift_names[s]; }
  string ShiftDescription(int s) const;

  int Weight(int c) const { return weights[c]; }

  int FindAgeGroup(string age_group_name) const;
  int FindSurgeon(string surgeon_id) const;
  int FindShift(string shift_name) const;
  int FindRoom(string room_name) const;
  int FindOT(string ot_name) const;
  int FindPatient(string patient_id) const;
  int FindNurse(string nurse_id) const;

  bool Overlap (int p,int d1, int d2) const;

 private:
  int days;
  int shifts_per_day;
  int shifts;
  int patients;
  int optional;
  int skill_levels;
  int age_groups;
  int surgeons;
  int operating_theaters;
  int rooms;
  int nurses;
  int occupants;
  
  vector<Patient> patients_vect;
  vector<Surgeon> surgeons_vect;
  vector<Occupant> occupants_vect;
  vector<OperatingTheater> operating_theaters_vect;
  vector<Room> rooms_vect;
  vector<Nurse> nurses_vect;
  
  vector<string> shift_names, age_group_names;
  
  vector<vector<int>> available_nurses;  
  vector<vector<vector<int>>> room_day_fixed_list;
  vector<int> weights;
  vector<vector<vector<int>>> swappable;

  void ResizeDataStructures();

};
#endif
