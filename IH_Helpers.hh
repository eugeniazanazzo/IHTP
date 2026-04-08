// File IH_Helpers.hh
#ifndef IH_HELPERS_HH
#define IH_HELPERS_HH

#include "IH_Output.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;


/***************************************************************************
 * Solution Manager 
 ***************************************************************************/

class IH_SolutionManager : public SolutionManager<IH_Input,IH_Output,DefaultCostStructure<int>> 
{
public:
  IH_SolutionManager(const IH_Input &);
  void RandomState(IH_Output& out);   
  bool CheckConsistency(const IH_Output& st) const;
  void PrettyPrintOutput(const IH_Output& st, string filename) const;
protected:
}; 

//Violations costs
class IH_OvertimeSurgeon : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_OvertimeSurgeon(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_OvertimeSurgeon") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_OvertimeOT : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_OvertimeOT(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_OvertimeOT") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_OverCapacity: public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_OverCapacity(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_OverCapacity") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_GenderMix : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_GenderMix(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_GenderMix") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_Unscheduled : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_Unscheduled(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_Unscheduled") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_Delay : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_Delay(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_Delay") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_OpenOT : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_OpenOT(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_OpenOT") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class  IH_AgeMix: public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_AgeMix(const IH_Input & in, int w, bool hard) : CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_AgeMix") 
  {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_Skill : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_Skill(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_Skill") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_Excess : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_Excess(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_Excess") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_Continuity : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_Continuity(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_Continuity") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

class IH_SurgeonTransfer : public CostComponent<IH_Input,IH_Output,int> 
{
public:
  IH_SurgeonTransfer(const IH_Input & in, int w, bool hard) :    CostComponent<IH_Input,IH_Output,int>(in,w,hard,"IH_SurgeonTransfer") {}
  int ComputeCost(const IH_Output& st) const override;
  void PrintViolations(const IH_Output& st, ostream& os = cout) const override;
};

//Moves
class IH_MoveAdmission
{
  friend bool operator==(const IH_MoveAdmission& m1, const IH_MoveAdmission& m2);
  friend bool operator!=(const IH_MoveAdmission& m1, const IH_MoveAdmission& m2);
  friend bool operator<(const IH_MoveAdmission& m1, const IH_MoveAdmission& m2);
  friend ostream& operator<<(ostream& os, const IH_MoveAdmission& mv);
  friend istream& operator>>(istream& is, IH_MoveAdmission& mv);
 public:
  int patient=-1, new_ad=-1, new_r=-1,new_ot=-1,ind=-1;
};

class IH_ChangeOT
{
  friend bool operator==(const IH_ChangeOT& m1, const IH_ChangeOT& m2);
  friend bool operator!=(const IH_ChangeOT& m1, const IH_ChangeOT& m2);
  friend bool operator<(const IH_ChangeOT& m1, const IH_ChangeOT& m2);
  friend ostream& operator<<(ostream& os, const IH_ChangeOT& mv);
  friend istream& operator>>(istream& is, IH_ChangeOT& mv);
 public:
  int patient=-1, new_ot=-1; 
};

class IH_ChangeRoom
{
  friend bool operator==(const IH_ChangeRoom& m1, const IH_ChangeRoom& m2);
  friend bool operator!=(const IH_ChangeRoom& m1, const IH_ChangeRoom& m2);
  friend bool operator<(const IH_ChangeRoom& m1, const IH_ChangeRoom& m2);
  friend ostream& operator<<(ostream& os, const IH_ChangeRoom& mv);
  friend istream& operator>>(istream& is, IH_ChangeRoom& mv);
 public:
  int patient=-1, new_r=-1; 
};

class IH_ChangeNurse
{
  friend bool operator==(const IH_ChangeNurse& m1, const IH_ChangeNurse& m2);
  friend bool operator!=(const IH_ChangeNurse& m1, const IH_ChangeNurse& m2);
  friend bool operator<(const IH_ChangeNurse& m1, const IH_ChangeNurse& m2);
  friend ostream& operator<<(ostream& os, const IH_ChangeNurse& mv);
  friend istream& operator>>(istream& is, IH_ChangeNurse& mv);
 public:
  int sh=-1, r=-1, nn=-1,idx=-1; 
};

class IH_SwapPatients
{
  friend bool operator==(const IH_SwapPatients& m1, const IH_SwapPatients& m2);
  friend bool operator!=(const IH_SwapPatients& m1, const IH_SwapPatients& m2);
  friend bool operator<(const IH_SwapPatients& m1, const IH_SwapPatients& m2);
  friend ostream& operator<<(ostream& os, const IH_SwapPatients& mv);
  friend istream& operator>>(istream& is, IH_SwapPatients& mv);
 public:
  int p1=-1, p2=-1, day1=-1, day2=-1; 
};

//Neighboorhood explorers

class IH_MoveAdmissionNeighborhoodExplorer
  : public NeighborhoodExplorer<IH_Input,IH_Output,IH_MoveAdmission> 
{
public:
  IH_MoveAdmissionNeighborhoodExplorer(const IH_Input & pin, SolutionManager<IH_Input,IH_Output>& psm)  
    : NeighborhoodExplorer<IH_Input,IH_Output,IH_MoveAdmission>(pin, psm, "IH_MoveAdmissionNeighborhoodExplorer") {}  

  void RandomMove(const IH_Output&, IH_MoveAdmission&) const override;  
  bool FeasibleMove(const IH_Output&, const IH_MoveAdmission&) const override;  
  void MakeMove(IH_Output& out, const IH_MoveAdmission&) const override;  
  void FirstMove(const IH_Output&, IH_MoveAdmission&) const override;  
  bool NextMove(const IH_Output&, IH_MoveAdmission&) const override;

protected:                    
  void AnyFirstMove(const IH_Output&, IH_MoveAdmission&) const;  
  bool AnyNextMove(const IH_Output&, IH_MoveAdmission&) const;  
  void FirstRoom(const IH_Output&, IH_MoveAdmission&) const; 
  void FirstOT(const IH_Output&, IH_MoveAdmission&) const;
  void FirstAdmissionDate(const IH_Output&, IH_MoveAdmission&) const;
  bool NextAdmissionDate(const IH_Output&, IH_MoveAdmission&) const;
  bool NextRoom(const IH_Output&, IH_MoveAdmission&) const;
  bool NextShift(const IH_Output&, IH_MoveAdmission&) const;
  bool NextOT(const IH_Output&, IH_MoveAdmission&) const;
  void NextPair(const IH_Output&, IH_MoveAdmission&) const;
};

class IH_ChangeOTNeighborhoodExplorer
  : public NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeOT> 
{
public:
  IH_ChangeOTNeighborhoodExplorer(const IH_Input & pin, SolutionManager<IH_Input,IH_Output>& psm)  
    : NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeOT>(pin, psm, "IH_ChangeOTNeighborhoodExplorer") {} 
  void RandomMove(const IH_Output&, IH_ChangeOT&) const override;          
  bool FeasibleMove(const IH_Output&, const IH_ChangeOT&) const override;  
  void MakeMove(IH_Output&, const IH_ChangeOT&) const override;             
  void FirstMove(const IH_Output&, IH_ChangeOT&) const override;  
  bool NextMove(const IH_Output&, IH_ChangeOT&) const override;   
protected:
  void AnyFirstMove(const IH_Output&, IH_ChangeOT&) const;  
  bool AnyNextMove(const IH_Output&, IH_ChangeOT&) const;
  void FirstOT(const IH_Output&, IH_ChangeOT&) const;
  bool NextOT(const IH_Output&, IH_ChangeOT&) const;
};

class IH_ChangeRoomNeighborhoodExplorer
  : public NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeRoom> 
{
public:
  IH_ChangeRoomNeighborhoodExplorer(const IH_Input & pin, SolutionManager<IH_Input,IH_Output>& psm)  
    : NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeRoom>(pin, psm, "IH_ChangeRoomNeighborhoodExplorer") {} 
  void RandomMove(const IH_Output&, IH_ChangeRoom&) const override;          
  bool FeasibleMove(const IH_Output&, const IH_ChangeRoom&) const override;  
  void MakeMove(IH_Output&, const IH_ChangeRoom&) const override;             
  void FirstMove(const IH_Output&, IH_ChangeRoom&) const override;  
  bool NextMove(const IH_Output&, IH_ChangeRoom&) const override;   
protected:
  void AnyFirstMove(const IH_Output&, IH_ChangeRoom&) const;  
  bool AnyNextMove(const IH_Output&, IH_ChangeRoom&) const;
  void FirstRoom(const IH_Output&, IH_ChangeRoom&) const;
  bool NextRoom(const IH_Output&, IH_ChangeRoom&) const;
};

class IH_ChangeNurseNeighborhoodExplorer
  : public NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeNurse> 
{
public:
  IH_ChangeNurseNeighborhoodExplorer(const IH_Input & pin, SolutionManager<IH_Input,IH_Output>& psm)  
    : NeighborhoodExplorer<IH_Input,IH_Output,IH_ChangeNurse>(pin, psm, "IH_ChangeNurseNeighborhoodExplorer") {} 
  void RandomMove(const IH_Output&, IH_ChangeNurse&) const override;          
  bool FeasibleMove(const IH_Output&, const IH_ChangeNurse&) const override;  
  void MakeMove(IH_Output&, const IH_ChangeNurse&) const override;             
  void FirstMove(const IH_Output&, IH_ChangeNurse&) const override;  
  bool NextMove(const IH_Output&, IH_ChangeNurse&) const override;   
protected:
  void AnyFirstMove(const IH_Output&, IH_ChangeNurse&) const;  
  bool AnyNextMove(const IH_Output&, IH_ChangeNurse&) const;
  void FirstNurse(const IH_Output&, IH_ChangeNurse&) const;
  bool NextNurse(const IH_Output&, IH_ChangeNurse&) const;
  bool NextRoom(const IH_Output&, IH_ChangeNurse&) const;
  void FirstRoom(const IH_Output&, IH_ChangeNurse&) const;
  void FirstSRNCombo(const IH_Output&, IH_ChangeNurse&) const;
};

class IH_SwapPatientsNeighborhoodExplorer
  : public NeighborhoodExplorer<IH_Input,IH_Output,IH_SwapPatients> {
public:
  IH_SwapPatientsNeighborhoodExplorer(const IH_Input & pin, SolutionManager<IH_Input,IH_Output>& psm)  
    : NeighborhoodExplorer<IH_Input,IH_Output,IH_SwapPatients>(pin, psm, "IH_SwapPatientsNeighborhoodExplorer") {}
  void RandomMove(const IH_Output&, IH_SwapPatients&) const override;
  bool FeasibleMove(const IH_Output&, const IH_SwapPatients&) const override;
  void MakeMove(IH_Output&, const IH_SwapPatients&) const override;
  void FirstMove(const IH_Output&, IH_SwapPatients&) const override;
  bool NextMove(const IH_Output&, IH_SwapPatients&) const override;
protected: 
  void AnyFirstMove(const IH_Output&, IH_SwapPatients&) const;
  bool AnyNextMove(const IH_Output&, IH_SwapPatients&) const;
  void FindNextP2(const IH_Output&, IH_SwapPatients&) const;
  void FindNextPair(const IH_Output&, IH_SwapPatients&) const;
  };

//Deltas
class IH_MoveAdmissionDeltaOvertimeSurgeon
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaOvertimeSurgeon(const IH_Input & in, IH_OvertimeSurgeon& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaOvertimeSurgeon") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaOvertimeOT
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaOvertimeOT(const IH_Input & in, IH_OvertimeOT& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaOvertimeOT") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};


class IH_MoveAdmissionDeltaOverCapacity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaOverCapacity(const IH_Input & in, IH_OverCapacity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaOverCapacity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaGenderMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaGenderMix(const IH_Input & in, IH_GenderMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaGenderMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaUnscheduled
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaUnscheduled(const IH_Input & in, IH_Unscheduled& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaUnscheduled") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaDelay
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaDelay(const IH_Input & in, IH_Delay& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaDelay") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaOpenOT
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaOpenOT(const IH_Input & in, IH_OpenOT& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaOpenOT") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaAgeMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaAgeMix(const IH_Input & in, IH_AgeMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaAgeMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaSkill
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaSkill(const IH_Input & in, IH_Skill& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaSkill") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaExcess
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaExcess(const IH_Input & in, IH_Excess& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaExcess") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaContinuity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaContinuity(const IH_Input & in, IH_Continuity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaContinuity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

class IH_MoveAdmissionDeltaSurgeonTransfer
  : public DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>
{
public:
  IH_MoveAdmissionDeltaSurgeonTransfer(const IH_Input & in, IH_SurgeonTransfer& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_MoveAdmission,int>(in,cc,"IH_MoveAdmissionDeltaSurgeonTransfer") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_MoveAdmission& mv) const;
};

//ChangeOT deltas

class IH_ChangeOTDeltaOvertimeOT
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>
{
public:
  IH_ChangeOTDeltaOvertimeOT(const IH_Input & in, IH_OvertimeOT& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>(in,cc,"IH_ChangeOTDeltaOvertimeOT") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeOT& mv) const;
};

class IH_ChangeOTDeltaOpenOT
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>
{
public:
  IH_ChangeOTDeltaOpenOT(const IH_Input & in, IH_OpenOT& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>(in,cc,"IH_ChangeOTDeltaOpenOT") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeOT& mv) const;
};

class IH_ChangeOTDeltaSurgeonTransfer
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>
{
public:
  IH_ChangeOTDeltaSurgeonTransfer(const IH_Input & in, IH_SurgeonTransfer& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeOT,int>(in,cc,"IH_ChangeOTDeltaSurgeonTransfer") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeOT& mv) const;
};

//ChangeRoom deltas

class IH_ChangeRoomDeltaOverCapacity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaOverCapacity(const IH_Input & in, IH_OverCapacity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaOverCapacity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

class IH_ChangeRoomDeltaGenderMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaGenderMix(const IH_Input & in, IH_GenderMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaGenderMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

class IH_ChangeRoomDeltaAgeMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaAgeMix(const IH_Input & in, IH_AgeMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaAgeMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

class IH_ChangeRoomDeltaSkill
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaSkill(const IH_Input & in, IH_Skill& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaSkill") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

class IH_ChangeRoomDeltaExcess
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaExcess(const IH_Input & in, IH_Excess& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaExcess") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

class IH_ChangeRoomDeltaContinuity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>
{
public:
  IH_ChangeRoomDeltaContinuity(const IH_Input & in, IH_Continuity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeRoom,int>(in,cc,"IH_ChangeRoomDeltaContinuity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeRoom& mv) const;
};

//Deltas

class IH_ChangeNurseDeltaSkill
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>
{
public:
  IH_ChangeNurseDeltaSkill(const IH_Input & in, IH_Skill& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>(in,cc,"IH_ChangeNurseDeltaSkill") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeNurse& mv) const;
};

class IH_ChangeNurseDeltaExcess
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>
{
public:
  IH_ChangeNurseDeltaExcess(const IH_Input & in, IH_Excess& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>(in,cc,"IH_ChangeNurseDeltaExcess") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeNurse& mv) const;
};

class IH_ChangeNurseDeltaContinuity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>
{
public:
  IH_ChangeNurseDeltaContinuity(const IH_Input & in, IH_Continuity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_ChangeNurse,int>(in,cc,"IH_ChangeNurseDeltaContinuity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_ChangeNurse& mv) const;
};
//SwapPatients deltas

class IH_SwapPatientsDeltaOvertimeSurgeon
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
public:
  IH_SwapPatientsDeltaOvertimeSurgeon(const IH_Input & in, IH_OvertimeSurgeon& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaOvertimeSurgeon") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaOvertimeOT
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaOvertimeOT(const IH_Input & in, IH_OvertimeOT& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaOvertimeOT") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaOverCapacity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaOverCapacity(const IH_Input & in, IH_OverCapacity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaOverCapacity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaGenderMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaGenderMix(const IH_Input & in, IH_GenderMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaGenderMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaDelay
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaDelay(const IH_Input & in, IH_Delay& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaDelay") {}
  int ComputeDeltaCost(const IH_Output&, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaAgeMix
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaAgeMix(const IH_Input & in, IH_AgeMix& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaAgeMix") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaSkill
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaSkill(const IH_Input & in, IH_Skill& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaSkill") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaExcess
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaExcess(const IH_Input & in, IH_Excess& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientDeltaExcess") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaContinuity
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaContinuity(const IH_Input & in, IH_Continuity& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaContinuity") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};

class IH_SwapPatientsDeltaSurgeonTransfer
  : public DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>
{
  public:
  IH_SwapPatientsDeltaSurgeonTransfer(const IH_Input & in, IH_SurgeonTransfer& cc) 
    : DeltaCostComponent<IH_Input,IH_Output,IH_SwapPatients,int>(in,cc,"IH_SwapPatientsDeltaSurgeonTransfer") {}
  int ComputeDeltaCost(const IH_Output& st, const IH_SwapPatients& mv) const;
};


#endif

