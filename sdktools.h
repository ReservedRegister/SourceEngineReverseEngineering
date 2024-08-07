typedef uint64_t uint64;

typedef enum _fieldtypes
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)
	
	FIELD_VECTOR2D,			// 2 floats

	FIELD_TYPECOUNT,		// MUST BE LAST
} fieldtype_t;

template <int FIELD_TYPE>
class CDatamapFieldSizeDeducer
{
public:
	enum
	{
		SIZE = 0
	};

	static int FieldSize( )
	{
		return 0;
	}
};

#define DECLARE_FIELD_SIZE( _fieldType, _fieldSize )	\
	template< > class CDatamapFieldSizeDeducer<_fieldType> { public: enum { SIZE = _fieldSize }; static int FieldSize() { return _fieldSize; } };
#define FIELD_SIZE( _fieldType )	CDatamapFieldSizeDeducer<_fieldType>::SIZE
#define FIELD_BITS( _fieldType )	(FIELD_SIZE( _fieldType ) * 8)

DECLARE_FIELD_SIZE( FIELD_FLOAT,		sizeof(float) )
DECLARE_FIELD_SIZE( FIELD_STRING,		sizeof(int) )
DECLARE_FIELD_SIZE( FIELD_VECTOR,		3 * sizeof(float) )
DECLARE_FIELD_SIZE( FIELD_VECTOR2D,		2 * sizeof(float) )
DECLARE_FIELD_SIZE( FIELD_QUATERNION,	4 * sizeof(float))
DECLARE_FIELD_SIZE( FIELD_INTEGER,		sizeof(int))
DECLARE_FIELD_SIZE( FIELD_BOOLEAN,		sizeof(char))
DECLARE_FIELD_SIZE( FIELD_SHORT,		sizeof(short))
DECLARE_FIELD_SIZE( FIELD_CHARACTER,	sizeof(char))
DECLARE_FIELD_SIZE( FIELD_COLOR32,		sizeof(int))
DECLARE_FIELD_SIZE( FIELD_CLASSPTR,		sizeof(int))
DECLARE_FIELD_SIZE( FIELD_EHANDLE,		sizeof(int))
DECLARE_FIELD_SIZE( FIELD_EDICT,		sizeof(int))
DECLARE_FIELD_SIZE( FIELD_POSITION_VECTOR, 	3 * sizeof(float))
DECLARE_FIELD_SIZE( FIELD_TIME,			sizeof(float))
DECLARE_FIELD_SIZE( FIELD_TICK,			sizeof(int))
DECLARE_FIELD_SIZE( FIELD_MODELNAME,	sizeof(int))
DECLARE_FIELD_SIZE( FIELD_SOUNDNAME,	sizeof(int))
DECLARE_FIELD_SIZE( FIELD_INPUT,		sizeof(int))
#ifdef POSIX
// pointer to members under gnuc are 8bytes if you have a virtual func
DECLARE_FIELD_SIZE( FIELD_FUNCTION,		sizeof(uint64))
#else
DECLARE_FIELD_SIZE( FIELD_FUNCTION,		sizeof(int *))
#endif
DECLARE_FIELD_SIZE( FIELD_VMATRIX,		16 * sizeof(float))
DECLARE_FIELD_SIZE( FIELD_VMATRIX_WORLDSPACE,	16 * sizeof(float))
DECLARE_FIELD_SIZE( FIELD_MATRIX3X4_WORLDSPACE,	12 * sizeof(float))
DECLARE_FIELD_SIZE( FIELD_INTERVAL,		2 * sizeof( float) )  // NOTE:  Must match interval.h definition
DECLARE_FIELD_SIZE( FIELD_MODELINDEX,	sizeof(int) )
DECLARE_FIELD_SIZE( FIELD_MATERIALINDEX,	sizeof(int) )

#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore
#define FTYPEDESC_SAVE				0x0002		// This field is saved to disk
#define FTYPEDESC_KEY				0x0004		// This field can be requested and written to by string name at load time
#define FTYPEDESC_INPUT				0x0008		// This field can be written to by string name at run time, and a function called
#define FTYPEDESC_OUTPUT			0x0010		// This field propogates it's value to all targets whenever it changes
#define FTYPEDESC_FUNCTIONTABLE		0x0020		// This is a table entry for a member function pointer
#define FTYPEDESC_PTR				0x0040		// This field is a pointer, not an embedded object
#define FTYPEDESC_OVERRIDE			0x0080		// The field is an override for one in a base class (only used by prediction system for now)

// Flags used by other systems (e.g., prediction system)
#define FTYPEDESC_INSENDTABLE		0x0100		// This field is present in a network SendTable
#define FTYPEDESC_PRIVATE			0x0200		// The field is local to the client or server only (not referenced by prediction code and not replicated by networking)
#define FTYPEDESC_NOERRORCHECK		0x0400		// The field is part of the prediction typedescription, but doesn't get compared when checking for errors

#define FTYPEDESC_MODELINDEX		0x0800		// The field is a model index (used for debugging output)

#define FTYPEDESC_INDEX				0x1000		// The field is an index into file data, used for byteswapping. 

// These flags apply to C_BasePlayer derived objects only
#define FTYPEDESC_VIEW_OTHER_PLAYER		0x2000		// By default you can only view fields on the local player (yourself), 
													//   but if this is set, then we allow you to see fields on other players
#define FTYPEDESC_VIEW_OWN_TEAM			0x4000		// Only show this data if the player is on the same team as the local player
#define FTYPEDESC_VIEW_NEVER			0x8000		// Never show this field to anyone, even the local player (unusual)