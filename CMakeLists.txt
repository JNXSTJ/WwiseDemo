cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

project(MyEngine)

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif(WIN32)


set(SOURCE_FILES
    main.cpp
    audio.cpp
    audio.h
    gameobject.h
    gameobject.cpp
    Platform.h
    Platform.cpp
    ShadowMap.h 
    ShadowMap.cpp 
    SkinnedMeshApp.h
    SkinnedMeshApp.cpp
    Ssao.h 
    Ssao.cpp
    Common/Camera.h
    Common/Camera.cpp
    Common/d3dApp.h
    Common/d3dApp.cpp
    Common/d3dUtil.h
    Common/d3dUtil.cpp
    Common/d3dx12.h
    Common/DDSTextureLoader.h
    Common/DDSTextureLoader.cpp
    Common/GameTimer.h
    Common/GameTimer.cpp
    Common/GeometryGenerator.h
    Common/GeometryGenerator.cpp
    Common/MathHelper.h
    Common/MathHelper.cpp
    Common/UploadBuffer.h
    Common/FrameResource.h 
    Common/FrameResource.cpp 
    Common/LoadM3d.h 
    Common/LoadM3d.cpp 
    Common/SkinnedData.h 
    Common/SkinnedData.cpp

    SoundEngine/Common/AkFileLocationBase.cpp
    SoundEngine/Common/AkFileLocationBase.h
    SoundEngine/Common/AkFilePackage.cpp
    SoundEngine/Common/AkFilePackage.h
    SoundEngine/Common/AkFilePackageLUT.cpp
    SoundEngine/Common/AkFilePackageLUT.h
    SoundEngine/Common/AkGeneratedSoundBanksResolver.cpp
    SoundEngine/Common/AkGeneratedSoundBanksResolver.h
    SoundEngine/Common/AkJobWorkerMgr.cpp
    SoundEngine/Common/AkJobWorkerMgr.h
    SoundEngine/Common/AkMultipleFileLocation.cpp
    SoundEngine/Common/AkMultipleFileLocation.h

    SoundEngine/Win32/AkDefaultIOHookDeferred.cpp
    SoundEngine/Win32/AkDefaultIOHookDeferred.h
)

# 创建文件夹并将文件放入其中
source_group("Source Files" FILES 
    main.cpp
    Platform.cpp
    audio.cpp
    gameobject.cpp
    ShadowMap.cpp 
    SkinnedMeshApp.cpp
    )
source_group("SoundEngine/Common" FILES
            SoundEngine/Common/AkFileLocationBase.cpp
            SoundEngine/Common/AkFileLocationBase.h
            SoundEngine/Common/AkFilePackage.cpp
            SoundEngine/Common/AkFilePackage.h
            SoundEngine/Common/AkFilePackageLUT.cpp
            SoundEngine/Common/AkFilePackageLUT.h
            SoundEngine/Common/AkGenerateSoundBanksResolver.cpp
            SoundEngine/Common/AkGenerateSoundBanksResolver.h
            SoundEngine/Common/AkJobWorkerMgr.cpp
            SoundEngine/Common/AkJobWorkerMgr.h
            SoundEngine/Common/AkMultipleFileLocation.cpp
            SoundEngine/Common/AkMultipleFileLocation.h
            )
source_group("SoundEngine/Win32" FILES
            SoundEngine/Win32/AkDefaultIOHookDeferred.cpp
            SoundEngine/Win32/AkDefaultIOHookDeferred.h
            AkFileHelpers.h 
            AkPlatformProfileHooks.cpp
            stdafx.cpp 
            stdafx.h
)
source_group("Common" FILES 
            Common/Camera.h
            Common/Camera.cpp
            Common/d3dApp.h
            Common/d3dApp.cpp
            Common/d3dUtil.h
            Common/d3dUtil.cpp
            Common/d3dx12.h
            Common/DDSTextureLoader.h
            Common/DDSTextureLoader.cpp
            Common/GameTimer.h
            Common/GameTimer.cpp
            Common/GeometryGenerator.h
            Common/GeometryGenerator.cpp
            Common/MathHelper.h
            Common/MathHelper.cpp
            Common/UploadBuffer.h
            Common/FrameResource.h 
            Common/FrameResource.cpp
            Common/LoadM3d.h
            Common/LoadM3d.cpp 
            Common/SkinnedData.h
            Common/SkinnedData.cpp
)
source_group("Header Files" 
            Platform.h 
            Audio.h
            LoadM3d.h 
            ShadowMap.h 
            SkinnedData.h
            Ssao.h
            gameobject.h
            )

# 定义项目
add_executable(WwiseDemo WIN32 ${SOURCE_FILES})


target_link_libraries(WwiseDemo PRIVATE 
    "ws2_32"
    "Ak3DAudioBedMixerFX"
    "AkAudioInputSource"
    "AkAutobahn"
    "AkCompressorFX"
    "AkDelayFX"
    "AkExpanderFX"
    "AkFlangerFX"
    "AkGainFX"
    "AkGuitarDistortionFX"
    "AkHarmonizerFX"
    "AkMatrixReverbFX"
    "AkMemoryMgr"
    "AkMeterFX"
    "AkMusicEngine"
    "AkOpusDecoder"
    "AkParametricEQFX"
    "AkPeakLimiterFX"
    "AkPitchShifterFX"
    "AkRecorderFX"
    "AkReflectFX"
    "AkRoomVerbFX"
    "AkSilenceSource"
    "AkSineSource"
    "AkSoundEngine"
    "AkSoundSeedWindSource"
    "AkSoundSeedWooshSource"
    "AkSpatialAudio"
    "AkStereoDelayFX"
    "AkStreamMgr"
    "AkSynthOneSource"
    "AkTimeStretchFX"
    "AkToneSource"
    "AkTremoloFX"
    "AkVorbisDecoder"
    "AuroHeadphoneFX"
    "CommunicationCentral"
    "iZHybridReverbFX"
    "iZTrashBoxModelerFX"
    "iZTrashDelayFX"
    "iZTrashDistortionFX"
    "iZTrashDynamicsFX"
    "iZTrashFiltersFX"
    "iZTrashMultibandDistortionFX"
    "MasteringSuiteFX"
    "McDSPFutzBoxFX"
    "McDSPLimiterFX"
)

# ASSIMP
find_package(ASSIMP REQUIRED)
if (ASSIMP_FOUND)
    # target_include_directories(${XXX_INCLUDE_DIRS})
    # target_link_libraries(your_target_name ${XXX_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find XXX library")
endif()

# tracy
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(third_party/tracy)
target_link_libraries(WwiseDemo PUBLIC Tracy::TracyClient 
    ${ASSIMP_LIBRARIES})

# 指定库文件目录
target_link_directories(WwiseDemo PRIVATE 
    "C:\\Program Files (x86)\\Audiokinetic\\Wwise2024.1.0.8669\\SDK\\x64_vc170\\Debug\\lib"
    "C:\\Users\\tao\\vcpkg\\installed\\x64-windows\\lib"
    )

target_include_directories(WwiseDemo PRIVATE 
    "C:\\Program Files (x86)\\Audiokinetic\\Wwise2024.1.0.8669\\SDK\\include"
    "C:\\Program Files (x86)\\Audiokinetic\\Wwise2024.1.0.8669\\SDK\\samples\\SoundEngine\\Win32"
    "C:\\Program Files (x86)\\Audiokinetic\\Wwise2024.1.0.8669\\SDK\\samples\\SoundEngine\\Common"
    "C:\\Users\\tao\\vcpkg\\installed\\x64-windows\\include"
    "C:\\Users\\tao\\vcpkg\\installed\\x64-windows\\include\\python3.12"
    ${ASSIMP_INCLUDE_DIRS}
)


if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif()