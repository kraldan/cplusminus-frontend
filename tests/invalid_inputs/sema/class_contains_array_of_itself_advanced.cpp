// test that if a class method contains parameter which has array of the class as subtype, 
// the same array type is later caught if used for a field

struct S {
	void f(S (*arr)[5]) {} // param 'arr' could potentially add 'S[5]' type to valid types cache,
				// which would circumvent the following error
	S arr[5];	// error
};
