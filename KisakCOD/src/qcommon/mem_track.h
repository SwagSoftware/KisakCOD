#pragma once

struct meminfo_t // sizeof=0xA0
{                                       // ...
    int total;                          // ...
    int nonSwapTotal;                   // ...
    int nonSwapMinSpecTotal;            // ...
    int typeTotal[37];                  // ...
};

struct mem_track_t // sizeof=0x14
{                                       // ...
    const char* name;                   // ...
    const char* filename;
    int size;                           // ...
    int pos;                            // ...
    unsigned __int8 type;               // ...
    unsigned __int8 usageType;          // ...
    // padding byte
    // padding byte
};

struct mem_track_node_s // sizeof=0x20
{
    mem_track_t data;
    int project;
    mem_track_node_s* prev;
    mem_track_node_s* next;
};

void track_init();
void track_static_alloc_internal(void* ptr, int size, const char* name, int type);

#define TRACK_STATIC_ARR(glob, type) track_static_alloc_internal(glob, sizeof(glob), #glob, type)
#define TRACK_STATIC_DAT(glob, type) track_static_alloc_internal(&glob, sizeof(glob), #glob, type)

void __cdecl CG_track_init();
void __cdecl TRACK_g_memtrack();
void __cdecl G_track_init();
void __cdecl TRACK_memtrack();
void __cdecl track_addbasicinfo(meminfo_t* info, int type, int size);
void __cdecl track_z_commit(int size, int type);
void __cdecl track_physical_alloc(int size, const char* name, int type);
void __cdecl track_hunk_alloc(int size, int pos, const char* name, int type);
void __cdecl track_hunk_allocLow(int size, int pos, const char* name, int type);
void __cdecl track_set_hunk_size(int size);
void __cdecl track_hunk_ClearToMarkHigh(int mark);
void __cdecl track_hunk_ClearToMarkLow(int mark);
void __cdecl track_hunk_ClearToStart();
void __cdecl track_shutdown(int project);
void __cdecl track_getbasicinfo(meminfo_t* info);
void __cdecl track_addbasicmeminfo(meminfo_t* sum, meminfo_t* in);
void __cdecl track_PrintInfo();
int __cdecl mem_track_compare(unsigned int* elem1, unsigned int* elem2);
void __cdecl track_PrintAllInfo();
void __cdecl UI_track_init();
double __cdecl ConvertToMB(int bytes);
void __cdecl R_Track_Init();

void __cdecl track_z_alloc(int size, const char* name, int type, char* pos, int project, int overhead);

// KISAKTODO: Tracking stubs.
void TRACK_cg_main();
void TRACK_cg_camerashake();
void TRACK_cg_draw();
void TRACK_cg_compassfriendlies();
void TRACK_cg_predict();
void TRACK_CG_CollWorld();
void TRACK_cg_view();
void TRACK_cg_localents();
void TRACK_cg_main();
void CL_ResetStats_f();
void TRACK_aim_assist();
void TRACK_aim_target();
void TRACK_g_main();
void TRACK_g_utils();
void TRACK_bg_misctables();
void TRACK_bg_weapons();
void TRACK_bg_weapons_load_obj();
void TRACK_g_combat();
void TRACK_g_scr_main();
void TRACK_g_mover();
void TRACK_g_hudelem();
void TRACK_bg_animation_mp();
void TRACK_r_bsp_load_obj();
void TRACK_rb_sky();
void TRACK_r_buffers();
void TRACK_r_debug();
void TRACK_r_dpvs();
void TRACK_r_font();
void TRACK_r_image();
void TRACK_r_image_wavelet();
void TRACK_r_init();
void TRACK_r_material();
void TRACK_r_model();
void TRACK_r_rendercmds();
void TRACK_r_scene();
void TRACK_r_screenshot();
void TRACK_r_staticmodelcache();
void TRACK_r_water();
void TRACK_r_workercmds();
void TRACK_rb_debug();
void TRACK_rb_backend();
void TRACK_rb_drawprofile();
void TRACK_rb_showcollision();
void TRACK_rb_state();
void TRACK_rb_stats();
void TRACK_rb_sunshadow();
void TRACK_ui_main();
void TRACK_ui_shared();
void TRACK_ui_utils();
void TRACK_ui_shared_obj();
void TRACK_scr_memorytree();
void TRACK_scr_stringlist();
void TRACK_scr_variable();
void TRACK_scr_vm();
void TRACK_scr_debugger();
void TRACK_scr_evaluate();
void TRACK_scr_compiler();
void TRACK_scr_parser();
void TRACK_xanim();
void TRACK_xmodel();
void TRACK_sv_main();
void TRACK_cm_load();
void TRACK_cm_showcollision();
void TRACK_cm_world();
void TRACK_cmd();
void TRACK_com_files();
void TRACK_com_math();
void TRACK_com_sndalias();
void TRACK_fx_random();
void TRACK_fx_system();
void TRACK_fx_marks();
void TRACK_dvar();
void TRACK_dvar_cmds();
void TRACK_msg();
void TRACK_profile();
void TRACK_profile_display();
void TRACK_q_parse();
void TRACK_win_common();
void TRACK_win_syscon();
void TRACK_win_net();
void TRACK_q_shared();
void TRACK_common();
void TRACK_dobj_management();
void TRACK_files();
void TRACK_scr_animtree();
void TRACK_com_memory();
void TRACK_sv_game();
void TRACK_memfile();
void TRACK_cl_main();
void TRACK_cl_console();
void TRACK_cl_input();
void TRACK_cl_keys();
void TRACK_cl_parse();
void TRACK_win_input();
void TRACK_cl_cgame();
void TRACK_statmonitor();
void TRACK_snd();
void TRACK_snd_driver();
void TRACK_stringed_hooks();
void TRACK_devgui();
void TRACK_assertive();
void TRACK_db_registry();
void TRACK_com_profilemapload();
void TRACK_record_dsound();
void TRACK_phys();
void TRACK_phys_ode();
void TRACK_profileMem();
void TRACK_missile_attractors();
void TRACK_DynEntityCollWorld();
void TRACK_ragdoll();

void __cdecl track_temp_alloc(int size, int hunkSize, int permanent, const char* name);
void __cdecl track_temp_free(int size, int permanent, const char* name);

void __cdecl track_temp_high_clear(int permanent);
void __cdecl track_temp_high_alloc(int size, int hunkSize, int permanent, const char* name);