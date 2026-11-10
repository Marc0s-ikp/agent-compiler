#pragma once
#include <ostream>

#include "abi.hpp"
#include "context.hpp"

// Moving struct and union values across a call (D19). Each function is half
// of a pair: the caller's and the callee's code both read the placement from
// ClassifyArgs / ClassifyReturn (abi.hpp), so they cannot disagree.

// Caller, before the call: the argument, saved as a copy in the temp slot at
// s0 + slot, goes to its registers or outgoing stack words. A struct over 8
// bytes is passed as the address of that copy.
void EmitPlaceRecordArg(std::ostream& os, const ArgLoc& loc, const Type& record,
                        int slot);

// Callee, at entry: the incoming parameter is stored into its home slot at
// s0 + home (a by-reference struct is copied there from the pointer).
void EmitStoreRecordParam(std::ostream& os, const ArgLoc& loc,
                          const Type& record, int home);

// Callee, 'return value;': a0 holds the address of the value. Afterwards the
// return registers hold it, or it has been copied through the hidden pointer.
void EmitReturnRecord(std::ostream& os, const Context& ctx, const Type& record);

// Caller, after the call: the returned struct is stored into the slot at
// s0 + result, and a0 holds that slot's address, the value of the call.
void EmitSaveReturnedRecord(std::ostream& os, const Type& record, int result);
