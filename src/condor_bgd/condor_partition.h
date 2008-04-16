#ifndef CONDOR_PARTITION_H
#define CONDOR_PARTITION_H

class MyString;
class ClassAd;

enum PState
{
	PSTATE_ERROR = 0,

	NOT_GENERATED,	// partition has yet to be made

	GENERATED,		// partition is already generated 

	BOOTED,			// partition is generated and booted

	ASSIGNED,		// partition is generated, booted, and in the process of
					// having a startd back it.

	BACKED,			// part is generated, booted, and a known startd presents it

	PSTATE_END
};

enum PKind
{
	PKIND_ERROR = 0,
	SMP,				// 1 process per node
	DUAL,				// 2 processes per node
	VN,					// 4 procosses per node
	PKIND_END
};

/* This object represents an already generted partition and whether or
	not it is activated. */
class Partition
{
	public:
		Partition();
		~Partition();

		// The class owns the memory of the classad and destructs it when
		// this class gets deleted
		void attach(ClassAd *ad);

		// transfer ownership of the internal classad memory back to the caller.
		ClassAd* detach(void);

		// what is the canonical name the BG thinks this partition is called?
		void set_name(MyString &name);
		MyString get_name(void);

		// Which, if any, startd backs this partition?
		void set_backer(MyString &owner);
		MyString get_backer(void);

		void set_size(size_t size);
		size_t get_size(void);

		void set_pstate(PState pstate);
		void set_pstate(MyString pstate);
		PState get_pstate(void);

		void set_pkind(MyString pkind);
		void set_pkind(PKind pkind);
		PKind get_pkind(void);

		// dprintf out the structure.
		void dump(int flags);

		void boot(char *script, PKind pkind);
		void back(char *script);

	private:
		bool	m_initialized;
		ClassAd	*m_data;
};

#endif


