#include "ext_main.h"

uintptr_t server_dll;

void InitExtension()
{
	server_dll = (uintptr_t)GetModuleHandle("server.dll") + 0xC00;

	ApplyPatches();
	rootconsole->ConsolePrint("server.dll [%X] SynergyUtils loaded for Windows!\n\n", server_dll);
}

void ApplyPatches()
{
	uintptr_t meta_hook = server_dll + 0x004E09C6;
	uintptr_t offset = 0;
	DWORD currentProts = NULL;

	VirtualProtect((void*)meta_hook, 5, PAGE_EXECUTE_READWRITE, &currentProts);
	offset = (uintptr_t)Hooks::MountGamePathsHook - meta_hook - 5;
	*(uint32_t*)(meta_hook + 1) = offset;
	VirtualProtect((void*)meta_hook, 5, currentProts, NULL);
}

//__stdcall
uintptr_t Hooks::MountGamePathsHook(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2)
{
	pThreeArgProt pDynamicThreeArgProt;

	uintptr_t cvar = *(uint32_t*)(server_dll + 0x876F7C);

	pTwoArgProtThis pDynamicTwoArgProtThis;
	pDynamicTwoArgProtThis = (pTwoArgProtThis)(*(uint32_t*)(*(uint32_t*)(cvar)+0x34));
	uintptr_t returnVal = pDynamicTwoArgProtThis(cvar, (uintptr_t)"sv_content_optional");
	uintptr_t object = *(uint32_t*)(returnVal+0x1C);
	char* optional_tags =  (char*) ( *(uint32_t*)(object + 0x24) );
	rootconsole->ConsolePrint("Real Val: [%s]", optional_tags);

	if (optional_tags)
	{
		char* cpy_optional_tags = (char*)copy_val(" ", optional_tags, strlen(optional_tags) + 1);
		char* begin = strstr((char*)arg0, cpy_optional_tags);

		if (begin != NULL)
		{
			//Determine lengths
			size_t optional_tags_length = strlen(cpy_optional_tags);

			int space_counter = 0;
			size_t tag_offset = -1;

			for (size_t i = 0; i < strlen((char*)arg0); i++)
			{
				if (*(char*)(arg0 + i) == ' ') space_counter++;

				if (space_counter >= 2)
				{
					tag_offset = i+1;
					break;
				}
			}

			if (tag_offset != -1)
			{
				char* tag_section = (char*)(arg0 + tag_offset);

				size_t begin_size = begin - tag_section;
				size_t middle_plus_end_size = strlen(begin);

				void* start_cpy = malloc(begin_size + 1);
				memcpy(start_cpy, tag_section, begin_size);
				*(char*)((uintptr_t)start_cpy + begin_size) = 0;

				if (middle_plus_end_size != optional_tags_length)
				{
					size_t end_size = middle_plus_end_size - optional_tags_length;
					void* end_cpy = malloc(end_size + 1);
					memcpy(end_cpy, begin + optional_tags_length, end_size);
					*(char*)((uintptr_t)end_cpy + end_size) = 0;

					memcpy((void*)tag_section, cpy_optional_tags + 1, optional_tags_length-1);
					memcpy((void*)(tag_section + optional_tags_length - 1), (void*)" ", 1);

					memcpy((void*)(tag_section + optional_tags_length), start_cpy, begin_size);
					memcpy((void*)(tag_section + optional_tags_length + begin_size), end_cpy, end_size);
					*(char*)(tag_section + optional_tags_length + begin_size + end_size) = 0;

					free(end_cpy);
				}
				else
				{
					memcpy((void*)tag_section, cpy_optional_tags + 1, optional_tags_length - 1);
					memcpy((void*)(tag_section + optional_tags_length - 1), (void*)" ", 1);

					memcpy((void*)(tag_section + optional_tags_length), start_cpy, begin_size);
					*(char*)(tag_section + optional_tags_length + begin_size) = 0;
				}

				free(start_cpy);
			}
		}

		free(cpy_optional_tags);
	}

	rootconsole->ConsolePrint("[%s] [%s] [%s]", arg0, arg1, arg2);

	pDynamicThreeArgProt = (pThreeArgProt)(server_dll + 0x00536E20);
	return pDynamicThreeArgProt(arg0, arg1, arg2);
}