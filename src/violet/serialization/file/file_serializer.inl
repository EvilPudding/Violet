// ============================================================================

template <typename StreamSerializer>
vlt::File_Serializer<StreamSerializer>::File_Serializer(std::filebuf && _file) :
	file(std::move(_file)),
	stream(&file),
	stream_serializer(stream)
{
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
vlt::File_Serializer<StreamSerializer>::~File_Serializer()
{
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
vlt::unique_val<vlt::Serializer> vlt::File_Serializer<StreamSerializer>::create_segment(const char * label)
{
	return stream_serializer.create_segment(label);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_b8(const char * label, bool const value)
{
	stream_serializer.write_b8(label, value);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_u32(const char * label, u32 const value)
{
	stream_serializer.write_u32(label, value);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_s32(const char * label, s32 const value)
{
	stream_serializer.write_s32(label, value);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_r32(const char * label, r32 const value)
{
	stream_serializer.write_r32(label, value);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_r64(const char * label, r64 const value)
{
	stream_serializer.write_r64(label, value);
}

// ----------------------------------------------------------------------------

template <typename StreamSerializer>
void vlt::File_Serializer<StreamSerializer>::write_string(const char * label, const char * value)
{
	stream_serializer.write_string(label, value);
}

// ============================================================================
