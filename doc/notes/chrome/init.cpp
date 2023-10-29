// /base/process/launch_posix.cc
Process LaunchProcess(const CommandLine &cmdline, const LaunchOptions &options) {
  return LaunchProcess(cmdline.argv(), options);
}

Process LaunchProcess(const std::vector<std::string>& argv, const LaunchOptions& options) {
	pid_t pid;
	std::vector<char*> argv_cstr;
	
	argv_cstr.reserve(argv.size() + 1);
  
	for (const auto& arg : argv)
		argv_cstr.push_back(const_cast<char*>(arg.c_str()));
	argv_cstr.push_back(nullptr);

	std::unique_ptr<char *[]> new_environ;
	char *const  empty_environ = nullptr;
	char *const* old_environ   = GetEnvironment();
  
	if (options.clear_environment)
    old_environ = &empty_environ;
	const char* current_directory = nullptr;
	if (!options.current_directory.empty())
		current_directory = options.current_directory.value().c_str();
 
	pid = ForkWithFlags(options.clone_flags | SIGCHLD, nullptr, nullptr); // || pid = fork()
	prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) 

	// Signal Chrome Elf that Chrome has begun to start.
	SignalChromeElf();
	MainDllLoader *loader = MakeMainDllLoader();
	int rc = loader->Launch(instance, exe_entry_point_ticks);
	loader->RelaunchChromeBrowserWithNewCommandLineIfNeeded();
	delete loader;
	


// DllMain
// -------
// Warning: The OS loader lock is held during DllMain.  Be careful.
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn633971.aspx
//
// - Note: Do not use install_static::GetUserDataDir from inside DllMain.
//         This can result in path expansion that triggers secondary DLL loads,
//         that will blow up with the loader lock held.
//         https://bugs.chromium.org/p/chromium/issues/detail?id=748949#c18
// -------------------------------------
// chrome/chrome_elf/chrome_elf_main.cc
// -------------------------------------
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    install_static::InitializeProductDetailsForPrimaryModule();
    install_static::InitializeProcessType();

    if (install_static::IsBrowserProcess()) {
      __try {
        // Disable third party extension points.
        elf_security::EarlyBrowserSecurity();

        // Initialize the blocking of third-party DLLs if the initialization of
        // the safety beacon succeeds.
        if (third_party_dlls::LeaveSetupBeacon())
          third_party_dlls::Init();
      } __except (elf_crash::GenerateCrashDump(GetExceptionInformation())) {
      }
    } else if (!install_static::IsCrashpadHandlerProcess()) {
      SignalInitializeCrashReporting();
      // CRT on initialization installs an exception filter which calls
      // TerminateProcess. We need to hook CRT's attempt to set an exception.
      elf_crash::DisableSetUnhandledExceptionFilter();
    }

  } else if (reason == DLL_PROCESS_DETACH) {
    elf_crash::ShutdownCrashReporting();
  }
  return TRUE;
}


// chrome/app/chrome_exe_main_win.cc
__declspec(dllexport) __cdecl void GetPakFileHashes(
    const uint8_t** resources_pak,
    const uint8_t** chrome_100_pak,
    const uint8_t** chrome_200_pak) {
  *resources_pak  = kSha256_resources_pak.data();
  *chrome_100_pak = kSha256_chrome_100_percent_pak.data();
  *chrome_200_pak = kSha256_chrome_200_percent_pak.data();
}


// chrome/app/chrome_exe_main_win.cc
#if !defined(WIN_CONSOLE_APP)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE prev, wchar_t*, int) {
#else   // !defined(WIN_CONSOLE_APP)
int main() {
  HINSTANCE instance = GetModuleHandle(nullptr);
#endif  // !defined(WIN_CONSOLE_APP)

	SetCwdForBrowserProcess();
	install_static::InitializeFromPrimaryModule();
	SignalInitializeCrashReporting();  
	if (IsBrowserProcess())
		chrome::DisableDelayLoadFailureHooksForMainExecutable();
	base::EnableTerminationOnOutOfMemory();
  
	// Initialize the CommandLine singleton from the environment.  
	base::CommandLine::Init(0, nullptr);
	const base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
	const std::string process_type        = command_line->GetSwitchValueASCII(switches::kProcessType);


	// Load and launch the chrome dll. *Everything* happens inside.
	VLOG(1) << "About to load main DLL.";
	MainDllLoader *loader = MakeMainDllLoader();
	int rc = loader->Launch(instance, exe_entry_point_ticks);
	loader->RelaunchChromeBrowserWithNewCommandLineIfNeeded();
	delete loader;
}


#ifdef WIN
DLLEXPORT int __cdecl ChromeMain(HINSTANCE instance,sandbox::SandboxInterfaceInfo* sandbox_info,int64_t exe_entry_point_ticks) {
#else
int ChromeMain(int argc, const char** argv) {
	int64_t exe_entry_point_ticks = 0;
#endif

#ifdef WIN
	allocator_shim::ConfigurePartitionAlloc();
	base::debug::HandleHooks::AddIATPatch(CURRENT_MODULE());
#endif

	ChromeMainDelegate chrome_main_delegate(base::TimeTicks::FromInternalValue(exe_entry_point_ticks));
	content::ContentMainParams params(&chrome_main_delegate);

#ifdef WIN
	params.instance     = instance;
	params.sandbox_info = sandbox_info;
#else
	params.argc = argc;
	params.argv = argv;
	base::CommandLine::Init(params.argc, params.argv);
#endif

	base::CommandLine::Init(0, nullptr);
	base::CommandLine *command_line(base::CommandLine::ForCurrentProcess());

}





44 	// This is the object which implements the zygote. The ZygoteMain function,
45 	// which is called from ChromeMain, at the the bottom and simple constructs one
46 	// of these objects and runs it.
47 	class Zygote {
48 	 public:
49 	  bool ProcessRequests() {
50 	    // A SOCK_SEQPACKET socket is installed in fd 3. We get commands from the
51 	    // browser on it.
52 	    // A SOCK_DGRAM is installed in fd 4. This is the sandbox IPC channel.
53 	    // See http://code.google.com/p/chromium/wiki/LinuxSandboxIPC
54 	
55 	    // We need to accept SIGCHLD, even though our handler is a no-op because
56 	    // otherwise we cannot wait on children. (According to POSIX 2001.)
57 	    struct sigaction action;
58 	    memset(&action, 0, sizeof(action));
59 	    action.sa_handler = SIGCHLDHandler;
60 	    CHECK(sigaction(SIGCHLD, &action, NULL) == 0);
61 	
62 	    for (;;) {
63 	      if (HandleRequestFromBrowser(3))
64 	        return true;
65 	    }
66 	  }
67 	