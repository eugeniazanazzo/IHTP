// File IH_Output.hh
#ifndef IH_OUTPUT_HH
#define IH_OUTPUT_HH

#include "IH_Input.hh"

class IH_Output
{
  friend ostream& operator<<(ostream& os, const IH_Output& out);
  friend istream& operator>>(istream& is, IH_Output& out);
  friend bool operator==(const IH_Output& out1, const IH_Output& out2);
 public:
  IH_Output(const IH_Output& out) = default;
  IH_Output& operator=(const IH_Output& out);
  IH_Output(const IH_Input& my_in);
  void AssignPatient(int p, int d, int r, int t);
  void AssignNurse(int n, int r, int s);
  void Reset();
  bool ScheduledPatient(int p) const { return admission_day[p] != -1; }
  int AdmissionDay(int p) const { return admission_day[p]; }
  int OT(int p) const{ return operating_room[p];}
  int Room(int p) const{ return room[p];}
  int NurseAttending(int r, int s) const { return room_shift_nurse[r][s]; }
  int SurgeonDayLoad(int s, int d) const { return surgeon_day_load[s][d]; }
  int OperatingTheaterDayLoad(int t, int d) const { return operatingtheater_day_load[t][d]; }
  int OperatingTheaterDayPatient(int t,int d,int i) const { return operatingtheater_day_patient_list[t][d][i];}
  bool TwoOTOpen (int d) const;
  int RoomDayBPatients(int r, int d) const { return room_day_b_patients[r][d]; }
  int RoomDayAPatients(int r, int d) const { return room_day_a_patients[r][d]; }
  int RoomDayLoad(int r, int d) const { return room_day_patient_list[r][d].size(); }
  int RoomDayPatient(int r, int d, int i) const { return room_day_patient_list[r][d][i]; }
  int NurseShiftLoad(int n, int s) const { return nurse_shift_load[n][s]; }
  int SurgeonDayTheaterCount(int s, int d, int t) const { return surgeon_day_theater_count[s][d][t]; }
  int SurgeonDayTheaters(int s,int d) const { return surgeon_day_theaters_visited[s][d]; }
  int Scheduled() const{ return scheduled; }
  int Assigned(int p,int n) const{return ever_assigned[p][n];}
  int PatientNumNurses(int p) const {return patient_num_nurses[p];}
  int FindPatientinOT(int p, int ot, int ad) const;
  int FindPatientinRoom(int p, int r, int d) const;
  int FindPatientinDailyAdmission(int p, int d) const;
  int FindRoominNurseList(int r, int n, int d) const;
  int AdmittedOnDay(int d) const{return daily_admissions[d].size();}
  int AdmittedPatientOnDay(int d,int i) const {return daily_admissions[d][i];}
  int RoomDayMin(int r,int d) const {return min_max[r][d].first;}
  int RoomDayMax(int r,int d) const {return min_max[r][d].second;}
  int RoomAgePres(int r,int d,int a) const {return room_day_age_distribution[r][d][a];}
  bool Admissable(int p,int d) const;
  vector<int>FindOverlappingNurses(int p,int nad,int nr) const;
  vector<int>FindCurrentOverlappingNurses(int p1,int p2) const;
  vector<int>FindFutureOverlappingNurses(int p1,int p2) const;
  int NotOverlappingSwapOvercapacity(int p1,int p2) const;
  int OverlappingSwapOvercapacity(int p1,int p2) const;
  int NotOverlappingSwapGenderMix(int p1,int p2) const;
  int OverlappingSwapGenderMix(int p1,int p2) const;
  int NotOverlappingSwapAgeMix(int p1,int p2) const;
  int OverlappingSwapAgeMix(int p1,int p2) const;
  int SwapSkill(int p1,int p2) const;
  int OverlappingSwapExcess(int p1,int p2) const;
  int InOutDeltaOvertimeSurgeon(int p1,int p2) const;
  int InOutDeltaOvertimeOT(int p1,int p2) const;

  //Setter
  void SetScheduled(int i) {scheduled=i;}

  int CountDistinctNurses(int p) const;
  int CountOccupantNurses(int o) const;

  int NewMin(int r,int d) const;
  int NewMax(int r,int d) const;
  int NewProjMin(int r,int d) const;
  int NewProjMax(int r,int d) const;

  void PrettyPrint(ostream& os,string info) const;

  void UpdatewithOccupantsInfo();
  void MoveAdmission(int p, int nad,int nar, int naot);
  void ChangeOT(int p,int ot);
  void ChangeRoom(int p,int r);
  void ChangeNurse(int sh,int r, int nn);
  void SwapOT(int p1,int p2);
  void SwapOTPatients(int p1,int p2);
  void SwapPatients(int p1,int p2);
  void SwapInOutPatients(int p1,int p2);
  void SwapRoomsPatients(int p1,int p2);
  void SwapNursesPatients(int p1,int p2);

  int DeltaOpenOT(int p,int n_d,int n_ot) const;
  int DeltaSurgeonOvertimeMP(int s,int d) const;

  int SwapGenders(int p,int d) const;
  int SubGender(int p,int d) const;
  int AddGender(int p,int d) const;
  int AddAgeGroup(int p,int d) const;
  int SubAgeGroup(int p,int d) const;
  int SwapAgeGroups(int p1,int p2,int d) const;

  int ReplaceSkill(int p1,int p2,int d) const;
  int SubSkill(int p,int d) const;
  int AddSkill(int p,int p2,int d) const;
  int SwapWorkloads(int p1,int p2,int sh) const;
  int AddWorkload(int p,int p1,int sh) const;
  int RemoveWorkload (int p,int sh) const;
  int WorkloadDelta(int n,int sh,int d) const;
  
 private:
  const IH_Input& in;
  int scheduled;
  vector<int> admission_day;  
  vector<int> room;
  vector<int> operating_room;
  vector<vector<int>> room_shift_nurse; 

  // patient data (redundant)
  vector<int> patient_num_nurses;
  vector<vector<int>> ever_assigned;
  // room data (redundant)
  vector<vector<vector<int>>> room_day_patient_list;
  vector<vector<vector<int>>> room_day_age_distribution;
  vector<vector<pair<int,int>>> min_max;
  vector<vector<int>> room_day_b_patients, room_day_a_patients;

  // nurse data (redundant)
  vector<vector<vector<int>>> nurse_shift_room_list;
  vector<vector<int>> nurse_shift_load;
  
  // operating theaters and surgeons (redundant)
  vector<vector<vector<int>>> operatingtheater_day_patient_list;
  vector<vector<int>> operatingtheater_day_load, surgeon_day_load;
  vector<vector<vector<int>>> surgeon_day_theater_count; 
  vector<vector<int>> surgeon_day_theaters_visited;
  vector<vector<int>> daily_admissions;
};
#endif