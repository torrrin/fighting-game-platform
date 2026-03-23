# web/CMakeLists.txt
if(EMSCRIPTEN)
    add_executable(game_wasm
        ../core/src/platform/game_registry.cpp
        ../core/src/platform/rollback_engine.cpp
        ../core/src/games/fighting_game.cpp
        export.cpp
    )
    
    target_include_directories(game_wasm PRIVATE
        ../core/include
    )
    
    target_link_options(game_wasm PRIVATE
        "SHELL:-s WASM=1"
        "SHELL:-s EXPORTED_FUNCTIONS=_malloc,_free,_game_create,_game_destroy,_game_update,_game_set_local_input,_game_receive_input,_game_get_render_data,_game_get_current_frame,_game_get_rollback_count"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,getValue,setValue,HEAPU8,HEAPU32,HEAPF32"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    )
    
    set_target_properties(game_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/web
    )
endif()