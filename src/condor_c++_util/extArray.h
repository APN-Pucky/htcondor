#ifndef __EXT_ARRAY_H__
#define __EXT_ARRAY_H__

#include "condor_debug.h"

template <class Element>
class ExtArray 
{
  public:

	// ctors/dtor
	ExtArray (int = 64); // initial size of array
	ExtArray (const ExtArray &);
	~ExtArray ();

	// accessor functions
	Element operator[] (int) const;
	Element &operator[] (int);

	// control functions
	int	getsize (void) const;
	int	getlast (void) const;
	Element *getarray (void);
	void resize (int);
	void fill (Element);

  private:

	Element *array;
	int 	size;
	int		last;
	Element filler;
};


template <class Element>
ExtArray<Element>::
ExtArray (int sz)
{
	// create array of required size
	size = sz;
	last = -1;
	array = new Element[size];
	if (!array)
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}
}

template <class Element>
ExtArray<Element>::
ExtArray (const ExtArray &old)
{
	int i;

	// sanity check
	if (&old == this) return;

	// establish new array of required size;
	size = old.size;
	last = old.last;
	array = new Element[size];
	if (!array) 
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}

	// copy elements over
	for (i = 0; i < size; i++) 
	{
		array[i] = old.array[i];
	}
	
	// copy filler element as well
	filler = old.filler;
}


template <class Element>
ExtArray<Element>::
~ExtArray ()
{
	delete [] array;
}


template <class Element>
inline Element ExtArray<Element>::
operator[] (int i) const
{
	// check bounds
	if (i < 0) 
	{
		i = 0;
	}
	else if (i >= size) 
	{
		return filler;
	}

	// index array
	return array[i];
}


template <class Element>
inline Element& ExtArray<Element>::
operator[] (int i)
{
	// check bounds ...
	if (i < 0) 
	{
		i = 0;
	}
	else 
	if (i >= size) 
	{
		resize (2*i);
	}

	// index array
	if( i > last ) last = i;
	return array[i];
}


template <class Element>
inline int ExtArray<Element>::
getsize (void) const
{
	return size;
}


template <class Element>
inline int ExtArray<Element>::
getlast (void) const
{
	return last;
}

template <class Element>
inline Element *ExtArray<Element>::
getarray (void) 
{
	return array;
}


template <class Element>
void ExtArray<Element>::
resize (int newsz) {
	Element *newarray = new Element[newsz];
	int		index = (size < newsz) ? size : newsz;

	// check if memory allocation was successful
	if (!newarray) 
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}

	// copy elements over to new array
	while (--index >= 0) 
	{
		newarray[index] = array[index];
	}

	// use new array
	delete [] array;
	size = newsz;	
	array = newarray;
}
		

template <class Element>
void ExtArray<Element>::
fill (Element elt)
{
	int i;
	for (i = 0; i < size; i++)
		array[i] = elt;
	filler = elt;
}


#endif//__EXT_ARRAY_H__
