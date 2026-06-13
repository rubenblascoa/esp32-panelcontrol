// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// This file is auto-extracted from web_pages.cpp.
#pragma once

#include <Arduino.h>

const char landing_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en" class="scroll-smooth"><head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Blasco OS - Multi-Program Execution Environment</title>
  <meta name="description" content="The ultimate multi-program execution environment for low-level engineering designed exclusively for the ESP32-S3.">
  <link href="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" rel="icon">
  
  
  
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&amp;family=Instrument+Serif:ital@1&amp;display=swap" rel="stylesheet">

  <style>
    .animate-on-scroll-hidden {
      opacity: 0;
      transform: translateY(20px);
      transition: opacity 0.8s cubic-bezier(0.2, 0.8, 0.2, 1), transform 0.8s cubic-bezier(0.2, 0.8, 0.2, 1);
    }
    .animate-on-scroll-visible {
      opacity: 1;
      transform: translateY(0);
    }
    .faq-item-open .faq-icon-vertical {
      transform: rotate(90deg);
    }
    
    html, body {
      overflow-x: hidden;
      max-width: 100vw;
    }

    .terminal-glow {
      text-shadow: 0 0 10px rgba(255, 69, 51, 0.5);
    }
  </style>

<style type="text/tailwindcss">@tailwind base;
@tailwind components;
@tailwind utilities;</style><script src="https://cdn.tailwindcss.com"></script><script>tailwind.config = {
  important: '#app-root',
  content: ["./_layout.html", "./content/**/*.html"],
  theme: {
    extend: {
      colors: {
        'brand-primary': 'rgb(255, 69, 51)',
        'neutral-background': 'rgb(10, 10, 10)',
        'neutral-surface': 'rgb(13, 13, 13)',
        'neutral-surface-alt': 'rgb(17, 17, 17)',
        'text-primary': 'rgb(255, 255, 255)',
        'text-secondary': 'rgba(255, 255, 255, 0.56)',
        'border-primary': 'rgba(255, 255, 255, 0.1)',
      },
      fontFamily: {
        'primary': ['Instrument Serif', 'serif'],
        'secondary': ['Inter', 'sans-serif'],
      },
      borderRadius: {
        'sm': '12px',
        'md': '16px',
        'lg': '24px',
        'xl': '32px',
        '2xl': '96px',
      },
      boxShadow: {
        'cta': '0 8px 25px rgba(255, 69, 51, 0.3)',
      },
      keyframes: {
        spin: {
          'from': { transform: 'translate(-50%, -50%) rotate(0deg)' },
          'to': { transform: 'translate(-50%, -50%) rotate(360deg)' },
        },
        'marquee-scroll': {
          'from': { transform: 'translateX(0)' },
          'to': { transform: 'translateX(-50%)' },
        },
      },
      animation: {
        'spin-slow': 'spin 10s linear infinite',
        'spin-reverse': 'spin 12s linear infinite reverse',
        'marquee': 'marquee-scroll 30s linear infinite',
        'text-marquee': 'marquee-scroll 25s linear infinite',
        'team-scroll': 'marquee-scroll 40s linear infinite',
        'testimonials-scroll': 'marquee-scroll 50s linear infinite',
      }
    },
  },
  plugins: [],
}</script></head>
<body id="app-root">

  <div class="bg-neutral-background text-text-primary font-secondary antialiased overflow-x-hidden min-h-screen pt-20 sm:pt-24">

  <svg aria-hidden="true" style="position: absolute; width: 0; height: 0; overflow: hidden;" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
    <defs>
      <symbol id="logo-icon" viewBox="0 0 24 24"><path d="M12 0L0 6v12l12 6 12-6V6L12 0zm0 2.309l9.333 4.667-9.333 4.667L2.667 6.976l9.333-4.667zm0 14.717l-9.333-4.667V7.662l9.333 4.667v8.947zm0 0l9.333-4.667V7.662l-9.333 4.667v8.947z"></path></symbol>
      <symbol id="cpu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2" ry="2"></rect><rect x="9" y="9" width="6" height="6"></rect><line x1="9" y1="1" x2="9" y2="4"></line><line x1="15" y1="1" x2="15" y2="4"></line><line x1="9" y1="20" x2="9" y2="23"></line><line x1="15" y1="20" x2="15" y2="23"></line><line x1="20" y1="9" x2="23" y2="9"></line><line x1="20" y1="15" x2="23" y2="15"></line><line x1="1" y1="9" x2="4" y2="9"></line><line x1="1" y1="15" x2="4" y2="15"></line></symbol>
      <symbol id="arrow-down-icon" viewBox="0 0 24 24"><path d="M7 10l5 5 5-5z"></path></symbol>
      <symbol id="cross-icon" viewBox="0 0 24 24"><path d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"></path></symbol>
      <symbol id="check-icon" viewBox="0 0 24 24"><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"></path></symbol>
    </defs>
  </svg>

  <header class="fixed top-0 left-0 w-full z-50 flex justify-center pt-3 sm:pt-6 pb-1 sm:pb-3">
    <nav id="main-nav" class="max-w-7xl w-full mx-auto px-2 sm:px-10 flex justify-center">
      <div id="nav-menu" class="bg-black/50 border border-border-primary rounded-sm backdrop-blur-md flex flex-row items-center gap-0.5 sm:gap-6 px-2 py-1.5 sm:px-8 sm:py-3 w-auto">
        <a href="#" class="flex items-center gap-1 shrink-0" data-pixel-id="mi8a7" data-pixel-kind="link">
            <img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="Logo" class="w-4 h-4 sm:w-8 sm:h-8 invert" data-pixel-id="eb5jh" data-pixel-kind="image">
        </a>
        <ul class="flex flex-row items-center gap-0 sm:gap-0.5 ml-0 sm:ml-2">
            <li data-pixel-id="ued6v" data-pixel-kind="text"><a href="#architecture" class="px-1.5 py-1 sm:px-3.5 sm:py-2 text-xs sm:text-sm font-medium hover:text-brand-primary transition-colors whitespace-nowrap" data-pixel-id="2l9tl" data-pixel-kind="link">Architecture</a></li>
            <li data-pixel-id="h49p5" data-pixel-kind="text"><a href="#features" class="px-1.5 py-1 sm:px-3.5 sm:py-2 text-xs sm:text-sm font-medium hover:text-brand-primary transition-colors whitespace-nowrap" data-pixel-id="qsnhc" data-pixel-kind="link">Features</a></li>
            <li data-pixel-id="99r5m" data-pixel-kind="text"><a href="#modules" class="px-1.5 py-1 sm:px-3.5 sm:py-2 text-xs sm:text-sm font-medium hover:text-brand-primary transition-colors whitespace-nowrap" data-pixel-id="2985k" data-pixel-kind="link">Modules</a></li>
            <li data-pixel-id="l5knv" data-pixel-kind="text"><a href="#faq" class="px-1.5 py-1 sm:px-3.5 sm:py-2 text-xs sm:text-sm font-medium hover:text-brand-primary transition-colors whitespace-nowrap" data-pixel-id="obcna" data-pixel-kind="link">FAQ</a></li>
        </ul>
        <a href="/login-page" target="_blank" class="ml-0.5 sm:ml-2 bg-brand-primary text-text-primary font-medium text-xs sm:text-sm px-2 py-1 sm:px-6 sm:py-3 rounded-sm hover:bg-red-700 transition-colors whitespace-nowrap" data-pixel-id="8xtmy" data-pixel-kind="link">Panel Control</a>
      </div>
    </nav>
  </header>

  <main><section class="relative min-h-screen flex items-center justify-center pt-8 sm:pt-16 pb-32 sm:pb-64 overflow-hidden">
    <div class="absolute left-1/2 bottom-[-200px] sm:bottom-[-400px] w-[800px] sm:w-[1200px] h-[500px] sm:h-[800px] transform -translate-x-1/2 z-[1]">
        <div class="absolute left-1/2 top-1/2 w-[250px] sm:w-[400px] h-[250px] sm:h-[400px] rounded-full bg-[conic-gradient(from_0deg_at_50%_50%,red,#ff001a,#00a6ff,#4797ff,#04f,#ff8000,red)] blur-[70px] animate-spin-slow"></div>
        <div class="absolute left-1/2 top-1/2 w-[200px] sm:w-[300px] h-[200px] sm:h-[300px] rounded-full bg-[conic-gradient(from_0deg_at_50%_50%,#ffd9ad,#139ce5,#fd864d)] blur-[32px] animate-spin-reverse"></div>
    </div>
    <div class="relative z-[2] max-w-7xl mx-auto px-4 sm:px-8 flex flex-col items-center gap-6 sm:gap-8 text-center">
        <p class="text-sm sm:text-base md:text-lg font-semibold -tracking-[0.02em] uppercase text-brand-primary px-4" data-animation-on-scroll="" data-animation-delay="100" data-pixel-id="6is3b" data-pixel-kind="text">ESP32-S3 EXCLUSIVE • LOW-LEVEL ENGINEERING</p>
        <h1 class="text-4xl sm:text-5xl md:text-6xl lg:text-7xl xl:text-[80px] font-medium -tracking-[0.04em] leading-tight max-w-5xl px-4" data-animation-on-scroll="" data-pixel-id="9rc43" data-pixel-kind="text">
            The ultimate <span class="font-primary font-normal italic">multi-program</span> runtime.
        </h1>
        <p class="text-base sm:text-lg md:text-xl font-medium -tracking-[0.02em] text-text-secondary max-w-2xl leading-relaxed px-4" data-animation-on-scroll="" data-animation-delay="200" data-pixel-id="srjc5" data-pixel-kind="text">
            Encapsulate and run multiple hardware projects on the same board. Wireless Telnet access, real-time telemetry, and dynamic hot configuration.
        </p>
        <div class="flex flex-col sm:flex-row gap-4 sm:gap-6 mt-4 w-full sm:w-auto px-4" data-animation-on-scroll="" data-animation-delay="400" data-pixel-id="q2msa" data-pixel-kind="text">
            <a href="https://github.com/rubenblascoa/esp32-panelcontrol" target="_blank" class="w-full sm:w-auto text-center inline-flex items-center justify-center px-6 py-3 rounded-sm font-medium text-base bg-brand-primary text-text-primary hover:bg-red-700 transition-all duration-300 transform hover:-translate-y-0.5 hover:shadow-cta" data-pixel-id="smic0" data-pixel-kind="link">Explore on GitHub</a>
            <a href="#features" class="w-full sm:w-auto text-center inline-flex items-center justify-center px-6 py-3 rounded-sm font-medium text-base bg-neutral-surface text-text-primary shadow-[0_0_0_1px_rgba(255,255,255,0.08)] hover:bg-white/10 transition-colors" data-pixel-id="o0vmi" data-pixel-kind="link">Core Features</a>
        </div>
    </div>
</section>

<!-- TECH MARQUEE -->
<section class="py-8 md:py-16 bg-neutral-background shadow-[0_0_32px_48px_theme(colors.neutral-background)] relative z-[2]">
    <div class="flex flex-col items-center gap-8 md:gap-12">
        <p class="font-medium text-text-secondary text-sm sm:text-base px-4 uppercase tracking-widest" data-pixel-id="0ejnz" data-pixel-kind="text">Technological Stack</p>
        <div class="w-full overflow-hidden [mask-image:linear-gradient(to_right,transparent,black_25%,black_75%,transparent)]">
            <div class="flex items-center gap-12 sm:gap-20 animate-text-marquee whitespace-nowrap marquee-content" data-pixel-id="hpavk" data-pixel-kind="text">
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">ESP32-S3</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">FreeRTOS</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">Telnet</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">WebSocket</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">LittleFS</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">C++ Native</span>
                <span class="text-xl sm:text-3xl font-bold tracking-[0.1em] text-brand-primary opacity-30 uppercase transition-opacity hover:opacity-80">NTP Sync</span>
            </div>
        </div>
    </div>
</section>

<!-- COMPARISON SECTION -->
<section id="comparison" class="py-16 md:py-32">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
        <div class="flex flex-col items-center gap-4 sm:gap-6 text-center mb-12 sm:mb-16" data-animation-on-scroll="">
                <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em]" data-pixel-id="0nsmi" data-pixel-kind="text">Problem vs Solution</span>
                <h2 class="text-3xl sm:text-4xl md:text-5xl lg:text-6xl font-medium -tracking-[0.04em] leading-tight max-w-4xl px-4" data-pixel-id="fi9jr" data-pixel-kind="text">Why <span class="font-primary font-normal italic">Blasco OS?</span></h2>
        </div>
        <div class="flex flex-col lg:flex-row justify-center items-center lg:items-start gap-6 sm:gap-8 lg:gap-12" data-animation-on-scroll="">
            <div class="flex flex-col items-center gap-6 sm:gap-8 w-full max-w-[450px]">
                <h3 class="text-2xl sm:text-3xl font-medium -tracking-[0.02em] text-zinc-600" data-pixel-id="p4vi7" data-pixel-kind="text">Standard Setup</h3>
                <ul class="w-full p-6 sm:p-8 border border-border-primary rounded-lg flex flex-col gap-4 sm:gap-6 bg-neutral-surface">
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-secondary" data-pixel-id="n9nwc" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0" fill="currentColor"><use href="#cross-icon"></use></svg><span>Mandatory USB connection</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-secondary" data-pixel-id="noez5" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0" fill="currentColor"><use href="#cross-icon"></use></svg><span>Flash for every pin change</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-secondary" data-pixel-id="dxf73" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0" fill="currentColor"><use href="#cross-icon"></use></svg><span>Basic Serial monitoring</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-secondary" data-pixel-id="8bnbx" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0" fill="currentColor"><use href="#cross-icon"></use></svg><span>Wired updates only</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-secondary" data-pixel-id="voa6x" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0" fill="currentColor"><use href="#cross-icon"></use></svg><span>Coupled projects break code</span></li>
                </ul>
            </div>
            <div class="flex flex-col items-center gap-6 sm:gap-8 w-full max-w-[450px]">
                <h3 class="text-2xl sm:text-3xl font-medium -tracking-[0.02em] flex items-center gap-3">
                    <img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="Icon" class="w-8 h-8 invert" data-pixel-id="b8vy1" data-pixel-kind="image">
                    Blasco OS
                </h3>
                <ul class="relative w-full p-6 sm:p-8 border border-border-primary rounded-lg flex flex-col gap-4 sm:gap-6 bg-neutral-background overflow-hidden">
                    <div class="absolute w-[500px] h-[500px] rounded-full bg-[conic-gradient(from_0deg_at_50%_50%,#ffd9ad,#139ce5,#fd864d)] blur-[32px] animate-spin-reverse top-[-250px] right-[-250px] transform rotate-[228deg]"></div>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-primary z-[1]" data-pixel-id="20zje" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0 text-white"><use href="#check-icon"></use></svg><span>100% Wireless via Telnet</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-primary z-[1]" data-pixel-id="49tj0" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0 text-white"><use href="#check-icon"></use></svg><span>Runtime Config (Captive Portal)</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-primary z-[1]" data-pixel-id="bcozy" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0 text-white"><use href="#check-icon"></use></svg><span>Advanced Telemetry (RAM, CPU, Temp)</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-primary z-[1]" data-pixel-id="o0xeu" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0 text-white"><use href="#check-icon"></use></svg><span>Full OTA Support (Over-The-Air)</span></li>
                    <li class="flex items-center gap-2 text-sm sm:text-base md:text-lg font-medium -tracking-[0.02em] text-text-primary z-[1]" data-pixel-id="12irz" data-pixel-kind="text"><svg class="w-5 h-5 fill-current shrink-0 text-white"><use href="#check-icon"></use></svg><span>Independent Modular Architecture</span></li>
                </ul>
            </div>
        </div>
    </div>
</section>

<!-- ARCHITECTURE SECTION -->
<section id="architecture" class="py-16 md:py-24 bg-neutral-surface/30">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
        <div class="grid grid-cols-1 lg:grid-cols-2 gap-12 items-center">
            <div data-animation-on-scroll="">
                <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em] mb-6" data-pixel-id="sgifk" data-pixel-kind="text">Modular Structure</span>
                <h2 class="text-3xl sm:text-4xl md:text-5xl font-medium -tracking-[0.04em] leading-tight mb-6" data-pixel-id="u5vin" data-pixel-kind="text">Multicore <span class="font-primary font-normal italic">Asynchronous</span> Orchestration.</h2>
                <div class="space-y-6">
                    <div class="p-6 border border-border-primary bg-neutral-surface rounded-lg">
                        <h4 class="text-brand-primary font-semibold mb-2" data-pixel-id="mqr3m" data-pixel-kind="text">Task Core 0: Network &amp; Storage</h4>
                        <p class="text-sm text-text-secondary" data-pixel-id="er75v" data-pixel-kind="text">Handles network interfaces, LittleFS local storage, OTA flashing, and asynchronous WebSocket raw inputs.</p>
                    </div>
                    <div class="p-6 border border-border-primary bg-neutral-surface rounded-lg">
                        <h4 class="text-brand-primary font-semibold mb-2" data-pixel-id="98b8e" data-pixel-kind="text">Task Core 1: Physical Hardware</h4>
                        <p class="text-sm text-text-secondary" data-pixel-id="01q51" data-pixel-kind="text">Fully dedicated to physical operations. Evaluates command queues (cmdQueue), refreshes diagnostics LCD, and advances background sensors.</p>
                    </div>
                </div>
            </div>
            <div class="relative" data-animation-on-scroll="">
                <div class="aspect-square rounded-2xl bg-gradient-to-br from-brand-primary/20 to-transparent border border-border-primary p-8 flex flex-col justify-center">
                    <div class="space-y-4">
                        <div class="flex items-center gap-4 p-4 bg-white/5 rounded-md border border-white/5">
                            <span class="w-10 h-10 rounded bg-brand-primary/20 flex items-center justify-center text-brand-primary font-bold">1</span>
                            <div>
                                <p class="font-medium" data-pixel-id="660ac" data-pixel-kind="text">Orchestration Core</p>
                                <p class="text-xs text-text-secondary" data-pixel-id="42aqa" data-pixel-kind="text">main.ino manages physical entry and boot</p>
                            </div>
                        </div>
                        <div class="flex items-center gap-4 p-4 bg-white/5 rounded-md border border-white/5">
                            <span class="w-10 h-10 rounded bg-brand-primary/20 flex items-center justify-center text-brand-primary font-bold">2</span>
                            <div>
                                <p class="font-medium" data-pixel-id="jd5h9" data-pixel-kind="text">FreeRTOS Scheduler</p>
                                <p class="text-xs text-text-secondary" data-pixel-id="b42jn" data-pixel-kind="text">tareas.cpp controls hardware infinite loops</p>
                            </div>
                        </div>
                        <div class="flex items-center gap-4 p-4 bg-white/5 rounded-md border border-white/5">
                            <span class="w-10 h-10 rounded bg-brand-primary/20 flex items-center justify-center text-brand-primary font-bold">3</span>
                            <div>
                                <p class="font-medium" data-pixel-id="842dw" data-pixel-kind="text">REST API Routing</p>
                                <p class="text-xs text-text-secondary" data-pixel-id="ap2is" data-pixel-kind="text">19 operational endpoints for remote control</p>
                            </div>
                        </div>
                    </div>
                    <div class="absolute -right-4 -bottom-4 w-24 h-24 bg-brand-primary/40 blur-3xl rounded-full animate-pulse"></div>
                </div>
            </div>
        </div>
    </div>
</section>

<!-- PCBWAY SPONSOR SECTION -->
<section id="pcbway" class="py-16 md:py-32">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
        <div class="relative bg-neutral-surface border border-border-primary rounded-lg p-8 sm:p-16 overflow-hidden" data-animation-on-scroll="">
            <div class="absolute top-0 right-0 w-96 h-96 bg-brand-primary/5 rounded-full blur-3xl"></div>
            <div class="grid grid-cols-1 lg:grid-cols-2 gap-12 items-center relative z-10">
                <div>
                    <h3 class="text-sm font-bold text-brand-primary uppercase tracking-widest mb-4" data-pixel-id="z16g4" data-pixel-kind="text">Hardware Powered by</h3>
                    <a href="https://www.pcbway.com/" target="_blank" data-pixel-id="8kkhx" data-pixel-kind="link">
                        <img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" class="h-16 w-auto mb-8" style="filter: brightness(0) invert(1);" data-pixel-id="oqut6" data-pixel-kind="image">
                    </a>
                    <p class="text-lg text-text-secondary leading-relaxed mb-6" data-pixel-id="qq3sy" data-pixel-kind="text">
                        "Developing ESP32 Blasco OS requires hardware capable of supporting asynchronous execution of multiple programs in real-time. For the physical deployment, I trust PCBWay's manufacturing services."
                    </p>
                    <ul class="space-y-3">
                        <li class="flex items-center gap-3 text-sm font-medium" data-pixel-id="q2yyw" data-pixel-kind="text"><svg class="w-5 h-5 text-brand-primary"><use href="#check-icon"></use></svg> Signal Integrity (SPI &amp; I2C)</li>
                        <li class="flex items-center gap-3 text-sm font-medium" data-pixel-id="jchhl" data-pixel-kind="text"><svg class="w-5 h-5 text-brand-primary"><use href="#check-icon"></use></svg> Real-time Telemetry Dissipation</li>
                        <li class="flex items-center gap-3 text-sm font-medium" data-pixel-id="d2khl" data-pixel-kind="text"><svg class="w-5 h-5 text-brand-primary"><use href="#check-icon"></use></svg> Assembly Precision (PCBA)</li>
                    </ul>
                </div>
                <div class="grid grid-cols-2 gap-4">
                    <img src="https://images.pexels.com/photos/6755081/pexels-photo-6755081.jpeg?w=800&amp;h=600&amp;fit=crop" alt="High quality professional PCB macro shot" class="rounded-lg border border-border-primary object-cover h-full" data-pixel-id="77uy4" data-pixel-kind="image">
                    <img src="https://images.pexels.com/photos/7286029/pexels-photo-7286029.jpeg?w=800&amp;h=600&amp;fit=crop" alt="Electronics engineering laboratory workbench" class="rounded-lg border border-border-primary mt-8 object-cover h-full" data-pixel-id="z70c8" data-pixel-kind="image">
                </div>
            </div>
        </div>
    </div>
</section>

<!-- ACTIVE MODULES GRID -->
<section id="modules" class="py-16 md:py-24 relative z-[1]">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
        <div class="flex flex-col items-center gap-4 sm:gap-6 text-center mb-12 sm:mb-16" data-animation-on-scroll="">
            <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em]" data-pixel-id="krxqb" data-pixel-kind="text">Active Modules</span>
            <h2 class="text-3xl sm:text-4xl md:text-5xl lg:text-6xl font-medium -tracking-[0.04em] leading-tight max-w-4xl px-4" data-pixel-id="6xbmf" data-pixel-kind="text">Three projects, <span class="font-primary font-normal italic">one single board.</span></h2>
            <p class="text-base sm:text-lg md:text-xl font-medium -tracking-[0.02em] text-text-secondary max-w-3xl px-4" data-pixel-id="y1dcl" data-pixel-kind="text">The OS currently includes three integrated engineering modules ready for deployment.</p>
        </div>
        <div class="grid grid-cols-1 md:grid-cols-3 gap-6 max-w-7xl mx-auto" data-animation-on-scroll="">
            <!-- NFC MODULE -->
            <div class="relative bg-neutral-surface border border-border-primary rounded-lg p-8 flex flex-col gap-6 overflow-hidden group hover:border-brand-primary/50 transition-all">
                <div class="absolute top-0 right-0 w-32 h-32 bg-brand-primary/10 rounded-full blur-2xl"></div>
                <div class="w-12 h-12 bg-brand-primary/10 rounded-lg flex items-center justify-center text-brand-primary">
                    <svg class="w-7 h-7" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z"></path></svg>
                </div>
                <div>
                    <h3 class="text-xl font-bold mb-2" data-pixel-id="z326k" data-pixel-kind="text">NFC Cloning Station Pro</h3>
                    <p class="text-sm text-text-secondary leading-relaxed" data-pixel-id="5l1lt" data-pixel-kind="text">Deep MIFARE Classic 1K auditing and physical cloning on magic cards (CUID/FUID). Real-time block extraction.</p>
                </div>
                <ul class="text-xs space-y-2 mt-auto">
                    <li class="flex items-center gap-2" data-pixel-id="7vmcm" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Sector 0 Audit</li>
                    <li class="flex items-center gap-2" data-pixel-id="prkvx" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Hot Cloning Mode</li>
                </ul>
            </div>
            <!-- ULTRASONIC RADAR -->
            <div class="relative bg-neutral-surface border border-border-primary rounded-lg p-8 flex flex-col gap-6 overflow-hidden group hover:border-brand-primary/50 transition-all">
                <div class="w-12 h-12 bg-brand-primary/10 rounded-lg flex items-center justify-center text-brand-primary">
                    <svg class="w-7 h-7" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 7h8m0 0v8m0-8l-8 8-4-4-6 6"></path></svg>
                </div>
                <div>
                    <h3 class="text-xl font-bold mb-2" data-pixel-id="zqjiy" data-pixel-kind="text">Ultrasonic Radar (V3)</h3>
                    <p class="text-sm text-text-secondary leading-relaxed" data-pixel-id="dm424" data-pixel-kind="text">100% non-blocking telemetry driven by hardware interrupts (ISR). Configurable cyclic refresh rate.</p>
                </div>
                <ul class="text-xs space-y-2 mt-auto">
                    <li class="flex items-center gap-2" data-pixel-id="nwi0d" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Asynchronous Execution</li>
                    <li class="flex items-center gap-2" data-pixel-id="cm0nl" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Thermal Fault Tolerance</li>
                </ul>
            </div>
            <!-- CLIMATE MONITOR -->
            <div class="relative bg-neutral-surface border border-border-primary rounded-lg p-8 flex flex-col gap-6 overflow-hidden group hover:border-brand-primary/50 transition-all">
                <div class="w-12 h-12 bg-brand-primary/10 rounded-lg flex items-center justify-center text-brand-primary">
                    <svg class="w-7 h-7" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path></svg>
                </div>
                <div>
                    <h3 class="text-xl font-bold mb-2" data-pixel-id="55rd2" data-pixel-kind="text">Climate Monitor (V1)</h3>
                    <p class="text-sm text-text-secondary leading-relaxed" data-pixel-id="kqcpk" data-pixel-kind="text">Acquisition via native 1-Wire protocol (Bit-Banging). Optimized against environmental EM noise.</p>
                </div>
                <ul class="text-xs space-y-2 mt-auto">
                    <li class="flex items-center gap-2" data-pixel-id="d5mfr" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Humidity &amp; Temperature</li>
                    <li class="flex items-center gap-2" data-pixel-id="y5kbk" data-pixel-kind="text"><svg class="w-3 h-3 text-brand-primary"><use href="#check-icon"></use></svg> Auto Error Recovery</li>
                </ul>
            </div>
        </div>
    </div>
</section>

<!-- FEATURES SECTION (Bento) -->
<section id="features" class="py-16 md:py-24">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
        <div class="flex flex-col items-center gap-4 sm:gap-6 text-center mb-12 sm:mb-16" data-animation-on-scroll="">
            <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em]" data-pixel-id="0frki" data-pixel-kind="text">System Features</span>
            <h2 class="text-3xl sm:text-4xl md:text-5xl lg:text-6xl font-medium -tracking-[0.04em] leading-tight max-w-4xl px-4" data-pixel-id="7ymn2" data-pixel-kind="text">Telemetry <span class="font-primary font-normal italic">Next-Gen.</span></h2>
        </div>
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6" data-animation-on-scroll="">
            <div class="p-8 border border-border-primary bg-neutral-surface rounded-lg lg:col-span-2">
                <h3 class="text-2xl font-medium mb-4" data-pixel-id="ewsl6" data-pixel-kind="text">Dynamic Mapping (Plug &amp; Play)</h3>
                <p class="text-text-secondary leading-relaxed" data-pixel-id="ru5ab" data-pixel-kind="text">All hardware connections (RST, SS, TRIG, ECHO, DHT) can be fully reassigned from the Web Captive Portal without touching the source code. Instant hot configuration for various ESP32 boards.</p>
            </div>
            <div class="p-8 border border-border-primary bg-brand-primary/10 rounded-lg">
                <h3 class="text-2xl font-medium mb-4" data-pixel-id="qm1z1" data-pixel-kind="text">EMA Smoothing</h3>
                <p class="text-text-secondary leading-relaxed" data-pixel-id="ezmi6" data-pixel-kind="text">Exponential Moving Average algorithm (EMA 30/70) used to filter momentary CPU stress peaks for accurate core telemetry.</p>
            </div>
            <div class="p-8 border border-border-primary bg-neutral-surface rounded-lg">
                <h3 class="text-2xl font-medium mb-4" data-pixel-id="xke5f" data-pixel-kind="text">NTP-Timed DB</h3>
                <p class="text-text-secondary leading-relaxed" data-pixel-id="4uete" data-pixel-kind="text">Structured 10-column logging in CSV format every 2 hours via LittleFS, synchronized with atomic network clocks.</p>
            </div>
            <div class="p-8 border border-border-primary bg-neutral-surface rounded-lg lg:col-span-2">
                <h3 class="text-2xl font-medium mb-4" data-pixel-id="zftf0" data-pixel-kind="text">100% Wireless Control</h3>
                <p class="text-text-secondary leading-relaxed" data-pixel-id="i6gxi" data-pixel-kind="text">Full access to the system terminal and diagnostic output via standard Telnet (Port 23) and raw WebSockets. Integrated OTA support for remote firmware updates.</p>
            </div>
        </div>
    </div>
</section>

<!-- FAQ SECTION -->
<section id="faq" class="py-16 md:py-24 bg-neutral-background">
    <div class="max-w-3xl mx-auto px-4 sm:px-8" data-animation-on-scroll="">
        <div class="text-center mb-12">
             <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em] mb-4" data-pixel-id="vl73w" data-pixel-kind="text">FAQ</span>
             <h2 class="text-3xl sm:text-4xl font-medium -tracking-[0.04em]" data-pixel-id="qvqel" data-pixel-kind="text">Technical Questions</h2>
        </div>
        <div class="faq-item border-b border-border-primary">
            <button class="faq-question w-full flex justify-between items-center py-5 sm:py-6 text-left text-base sm:text-lg font-medium text-text-primary gap-4" data-pixel-id="j5oo9" data-pixel-kind="text">
                <span>How is the initial deployment handled?</span>
                <span class="faq-icon relative w-3 h-3 shrink-0"><span class="absolute top-[5px] left-0 w-3 h-0.5 bg-text-primary rounded-lg transition-transform duration-300"></span><span class="faq-icon-vertical absolute top-0 left-[5px] w-0.5 h-3 bg-text-primary rounded-lg transition-transform duration-300"></span></span>
            </button>
            <div class="faq-answer max-h-0 overflow-hidden transition-all duration-500 ease-in-out">
                <p class="text-sm sm:text-base text-text-secondary leading-relaxed pb-5 sm:pb-6" data-pixel-id="n3x9w" data-pixel-kind="text">Flash the source code via USB once. The ESP32 will open an Access Point named "Esp32BlascoOS_Setup". Connect via your smartphone to configure Wi-Fi and GPIOs without recompiling.</p>
            </div>
        </div>
        <div class="faq-item border-b border-border-primary">
            <button class="faq-question w-full flex justify-between items-center py-5 sm:py-6 text-left text-base sm:text-lg font-medium text-text-primary gap-4" data-pixel-id="qku9n" data-pixel-kind="text">
                <span>What is the recommended compilation setup?</span>
                <span class="faq-icon relative w-3 h-3 shrink-0"><span class="absolute top-[5px] left-0 w-3 h-0.5 bg-text-primary rounded-lg transition-transform duration-300"></span><span class="faq-icon-vertical absolute top-0 left-[5px] w-0.5 h-3 bg-text-primary rounded-lg transition-transform duration-300"></span></span>
            </button>
            <div class="faq-answer max-h-0 overflow-hidden transition-all duration-500 ease-in-out">
                <p class="text-sm sm:text-base text-text-secondary leading-relaxed pb-5 sm:pb-6" data-pixel-id="pgha6" data-pixel-kind="text">Optimized for ESP32-S3 (N16R8). Use the "16M Flash (3MB APP/9.9MB FATFS)" partition scheme and enable OPI PSRAM in the Arduino IDE Tools menu.</p>
            </div>
        </div>
        <div class="faq-item border-b border-border-primary">
            <button class="faq-question w-full flex justify-between items-center py-5 sm:py-6 text-left text-base sm:text-lg font-medium text-text-primary gap-4" data-pixel-id="39xsq" data-pixel-kind="text">
                <span>Why use FreeRTOS for this project?</span>
                <span class="faq-icon relative w-3 h-3 shrink-0"><span class="absolute top-[5px] left-0 w-3 h-0.5 bg-text-primary rounded-lg transition-transform duration-300"></span><span class="faq-icon-vertical absolute top-0 left-[5px] w-0.5 h-3 bg-text-primary rounded-lg transition-transform duration-300"></span></span>
            </button>
            <div class="faq-answer max-h-0 overflow-hidden transition-all duration-500 ease-in-out">
                <p class="text-sm sm:text-base text-text-secondary leading-relaxed pb-5 sm:pb-6" data-pixel-id="eknkw" data-pixel-kind="text">To ensure non-blocking asynchronous execution. Core 0 handles all networking while Core 1 is 100% dedicated to physical sensor control, preventing micro-freezes during data acquisition.</p>
            </div>
        </div>
    </div>
</section>

<!-- FINAL CTA -->
<section class="relative py-16 sm:py-24 overflow-hidden">
    <div class="absolute left-1/2 bottom-[-200px] sm:bottom-[-346px] w-[700px] sm:w-[951px] h-[500px] sm:h-[634px] transform -translate-x-1/2 z-[1]">
        <div class="absolute left-1/2 top-1/2 w-[300px] sm:w-[400px] h-[300px] sm:h-[400px] rounded-full bg-[conic-gradient(from_0deg_at_50%_50%,red,#ff001a,#00a6ff,#4797ff,#04f,#ff8000,red)] blur-[70px] opacity-80 animate-spin-slow"></div>
        <div class="absolute left-1/2 top-1/2 w-[200px] sm:w-[300px] h-[200px] sm:h-[300px] rounded-full bg-[conic-gradient(from_0deg_at_50%_50%,#ffd9ad,#139ce5,#fd864d)] blur-[32px] animate-spin-reverse"></div>
    </div>
    <div class="relative z-[2] max-w-7xl mx-auto px-4 sm:px-8">
        <div class="bg-black/40 border border-border-primary rounded-lg p-6 sm:p-8 md:p-12 flex flex-col items-center gap-4 sm:gap-6 text-center backdrop-blur-sm" data-animation-on-scroll="">
            <span class="inline-block py-2 px-3 border border-border-primary bg-neutral-surface-alt rounded-sm text-sm font-medium -tracking-[0.02em]" data-pixel-id="lr5ee" data-pixel-kind="text">Ready to hack?</span>
            <h2 class="text-3xl sm:text-4xl md:text-5xl lg:text-6xl font-medium -tracking-[0.04em] leading-tight max-w-xl px-2" data-pixel-id="z594e" data-pixel-kind="text">Deploy <span class="font-primary font-normal italic">Blasco OS</span> today.</h2>
            <p class="text-base sm:text-lg md:text-xl text-text-secondary max-w-2xl leading-relaxed px-2" data-pixel-id="fqlu4" data-pixel-kind="text">Join the project on GitHub, report bugs, or suggest improvements for the continuous kernel expansion.</p>
            <div class="flex flex-col sm:flex-row gap-4 sm:gap-6 mt-2 w-full sm:w-auto" data-pixel-id="bds20" data-pixel-kind="text">
                <a href="https://github.com/rubenblascoa/esp32-panelcontrol" target="_blank" class="w-full sm:w-auto text-center inline-flex items-center justify-center px-6 py-3 rounded-sm font-medium text-sm sm:text-base bg-brand-primary text-text-primary hover:bg-red-700 transition-colors" data-pixel-id="whuxx" data-pixel-kind="link">Explore Source Code</a>
                <a href="#architecture" class="w-full sm:w-auto text-center inline-flex items-center justify-center px-6 py-3 rounded-sm font-medium text-sm sm:text-base bg-neutral-surface text-text-primary shadow-[0_0_0_1px_rgba(255,255,255,0.08)] hover:bg-white/10 transition-colors" data-pixel-id="95e9f" data-pixel-kind="link">Documentation</a>
            </div>
        </div>
    </div>
</section></main>

  <footer class="relative z-[2] bg-black/50 border-t border-border-primary backdrop-blur-sm">
    <div class="max-w-7xl mx-auto px-4 sm:px-8">
      <div class="py-16">
        <div class="flex flex-col lg:flex-row justify-between items-center lg:items-start text-center lg:text-left gap-12">
          <div class="flex flex-col items-center lg:items-start gap-8">
            <a href="#" class="flex items-center gap-3 text-3xl font-medium -tracking-[0.05em]" data-pixel-id="u562m" data-pixel-kind="link">
              <img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="Logo" class="w-10 h-10 invert" data-pixel-id="hvt0w" data-pixel-kind="image">
              <span data-pixel-id="sv45y" data-pixel-kind="text">ESP32 <span class="font-primary font-normal italic">Blasco OS</span></span>
            </a>
            <div class="max-w-md">
              <h4 class="text-lg font-medium" data-pixel-id="zj7qn" data-pixel-kind="text">Contact &amp; Sponsoring</h4>
              <p class="text-sm text-text-secondary leading-relaxed mt-4 mb-4" data-pixel-id="dyoca" data-pixel-kind="text">Developed with passion by Ruben Blasco Armengod. Interested in sponsoring or collaborating?</p>
              <div class="flex flex-col gap-2">
                <a href="mailto:rubenblascoarmengod@gmail.com" class="text-brand-primary hover:underline" data-pixel-id="filkf" data-pixel-kind="link">rubenblascoarmengod@gmail.com</a>
                <div class="flex gap-4 mt-2 justify-center lg:justify-start" data-pixel-id="ygq89" data-pixel-kind="text">
                  <a href="https://github.com/rubenblascoa" class="hover:text-brand-primary transition-colors" data-pixel-id="ur80u" data-pixel-kind="link">GitHub</a>
                  <a href="https://instagram.com/rubenblascoa" class="hover:text-brand-primary transition-colors" data-pixel-id="0mvmn" data-pixel-kind="link">Instagram</a>
                </div>
              </div>
            </div>
          </div>
          <div class="flex flex-wrap justify-center gap-8 lg:gap-16">
            <div class="w-32 text-left">
              <h4 class="text-lg font-medium mb-4" data-pixel-id="oe5y1" data-pixel-kind="text">Project</h4>
              <ul class="flex flex-col gap-4">
                <li data-pixel-id="jgrh2" data-pixel-kind="text"><a href="#architecture" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="dhlhh" data-pixel-kind="link">Architecture</a></li>
                <li data-pixel-id="hej7u" data-pixel-kind="text"><a href="#features" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="jk2a0" data-pixel-kind="link">Features</a></li>
                <li data-pixel-id="wh7iv" data-pixel-kind="text"><a href="#modules" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="fly1o" data-pixel-kind="link">Modules</a></li>
                <li data-pixel-id="rhc0c" data-pixel-kind="text"><a href="https://github.com/rubenblascoa/esp32-panelcontrol/issues" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="0d396" data-pixel-kind="link">Report a Bug</a></li>
              </ul>
            </div>
            <div class="w-32 text-left">
              <h4 class="text-lg font-medium mb-4" data-pixel-id="lt3bi" data-pixel-kind="text">Hardware</h4>
              <ul class="flex flex-col gap-4">
                <li data-pixel-id="y0lhk" data-pixel-kind="text"><a href="https://www.pcbway.com/" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="mmzjp" data-pixel-kind="link">PCBWay</a></li>
                <li data-pixel-id="2n6pz" data-pixel-kind="text"><a href="#" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="u6je9" data-pixel-kind="link">ESP32-S3</a></li>
                <li data-pixel-id="j98o5" data-pixel-kind="text"><a href="#" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="8sxeo" data-pixel-kind="link">MFRC522</a></li>
                <li data-pixel-id="xb3wk" data-pixel-kind="text"><a href="#" class="text-sm text-text-secondary hover:text-text-primary transition-colors" data-pixel-id="3al94" data-pixel-kind="link">HC-SR04</a></li>
              </ul>
            </div>
          </div>
        </div>
      </div>
      <div class="border-t border-border-primary">
        <div class="flex flex-col sm:flex-row justify-between items-center py-6 gap-3">
          <p class="text-sm text-text-secondary" data-pixel-id="k05i6" data-pixel-kind="text">© 2026 ESP32 Blasco OS. Licensed under MIT.</p>
          <div class="flex items-center gap-2">
            <p class="text-sm text-text-secondary" data-pixel-id="gm7hn" data-pixel-kind="text">Built for low-level engineering.</p>
          </div>
        </div>
      </div>
    </div>
  </footer>

  </div>

  <script>
    document.addEventListener('DOMContentLoaded', () => {
      // --- MOBILE NAVIGATION ---
      const navToggle = document.getElementById('nav-toggle');
      const navMenuMobile = document.getElementById('nav-menu-mobile');
      const body = document.body;
      
      if (false) { const navLinks = []; navLinks.forEach(link => {
              link.addEventListener('click', () => {
                  if (window.innerWidth < 768) {
                      navMenu.classList.add('-translate-y-full');
                      navMenu.classList.remove('translate-y-0');
                      navToggle.querySelector('span:nth-child(1)').classList.remove('translate-y-[7.5px]', 'rotate-45');
                      navToggle.querySelector('span:nth-child(2)').classList.remove('opacity-0');
                      navToggle.querySelector('span:nth-child(3)').classList.remove('-translate-y-[7.5px]', '-rotate-45');
                      body.style.overflow = '';
                  }
              });
          });
      }

      // --- FAQ ACCORDION ---
      const faqItems = document.querySelectorAll('.faq-item');
      faqItems.forEach(item => {
          const question = item.querySelector('.faq-question');
          const answer = item.querySelector('.faq-answer');
          question.addEventListener('click', () => {
              const isOpen = item.classList.contains('faq-item-open');
              faqItems.forEach(otherItem => {
                  if (otherItem !== item) {
                      otherItem.classList.remove('faq-item-open');
                      otherItem.querySelector('.faq-answer').style.maxHeight = null;
                  }
              });
              if (isOpen) {
                  item.classList.remove('faq-item-open');
                  answer.style.maxHeight = null;
              } else {
                  item.classList.add('faq-item-open');
                  answer.style.maxHeight = answer.scrollHeight + 'px';
              }
          });
      });

      // --- SCROLLING MARQUEE ---
      document.querySelectorAll('.marquee-content').forEach(marquee => {
          const content = marquee.innerHTML;
          marquee.innerHTML += content;
      });

      // --- APPEAR ON SCROLL ANIMATIONS ---
      const animatedElements = document.querySelectorAll('[data-animation-on-scroll]');
      const observer = new IntersectionObserver((entries) => {
          entries.forEach(entry => {
              if (entry.isIntersecting) {
                  const delay = parseInt(entry.target.getAttribute('data-animation-delay')) || 0;
                  setTimeout(() => {
                      entry.target.classList.add('animate-on-scroll-visible');
                  }, delay);
                  observer.unobserve(entry.target);
              }
          });
      }, { threshold: 0.1 });
      animatedElements.forEach(el => {
        el.classList.add('animate-on-scroll-hidden');
        observer.observe(el);
      });
    });
  </script>


    <script src="https://api.landinghero.ai/public/assistant-widget.js" data-lh-assistant-widget="true" data-project-id="fad5krkVr9v4ZihoVNwE"></script>

    

    <script id="lh-website-form-handler" data-lh-form-handler="true">
    (function() {
      if (window.__lhWebsiteFormHandlerInitialized) return;
      window.__lhWebsiteFormHandlerInitialized = true;

      var PROJECT_ID = "fad5krkVr9v4ZihoVNwE";
      var DEFAULT_SUCCESS_MESSAGE = 'Thanks. Your message has been sent.';
      var DEFAULT_ERROR_MESSAGE = 'Could not submit the form. Please try again.';

      function isLocalHostname(hostname) {
        var normalized = String(hostname || '').trim().toLowerCase();
        return normalized === 'localhost' || normalized === '127.0.0.1';
      }

      function readHostnameFromUrl(url) {
        if (!url) return '';

        try {
          return String(new URL(url).hostname || '').trim().toLowerCase();
        } catch (_) {
          return '';
        }
      }

      function detectApiBaseUrl() {
        try {
          var hostname = window.location.hostname || '';
          if (isLocalHostname(hostname)) {
            return 'http://localhost:8080/api';
          }
        } catch (_) {}

        try {
          var baseHostname = readHostnameFromUrl(document.baseURI || '');
          if (isLocalHostname(baseHostname)) {
            return 'http://localhost:8080/api';
          }
        } catch (_) {}

        try {
          var parentHostname =
            window.parent && window.parent !== window && window.parent.location
              ? String(window.parent.location.hostname || '').trim().toLowerCase()
              : '';
          if (isLocalHostname(parentHostname)) {
            return 'http://localhost:8080/api';
          }
        } catch (_) {}

        return 'https://api.landinghero.ai/api';
      }

      var API_BASE_URL = detectApiBaseUrl();

      function getFeedbackNode(form) {
        var feedback = form.nextElementSibling;
        if (
          feedback &&
          feedback.nodeType === 1 &&
          feedback.getAttribute('data-lh-form-feedback') === 'true'
        ) {
          return feedback;
        }

        feedback = document.createElement('div');
        feedback.setAttribute('data-lh-form-feedback', 'true');
        feedback.setAttribute('aria-live', 'polite');
        feedback.style.marginTop = '12px';
        feedback.style.padding = '12px 14px';
        feedback.style.borderRadius = '10px';
        feedback.style.fontSize = '14px';
        feedback.style.lineHeight = '1.5';
        feedback.style.display = 'none';
        form.insertAdjacentElement('afterend', feedback);
        return feedback;
      }

      function setFeedback(form, kind, message) {
        var feedback = getFeedbackNode(form);
        feedback.textContent = message || '';
        feedback.style.display = message ? 'block' : 'none';
        feedback.style.border = kind === 'success'
          ? '1px solid rgba(16, 185, 129, 0.25)'
          : '1px solid rgba(239, 68, 68, 0.25)';
        feedback.style.background = kind === 'success'
          ? 'rgba(16, 185, 129, 0.08)'
          : 'rgba(239, 68, 68, 0.08)';
        feedback.style.color = kind === 'success' ? '#065f46' : '#991b1b';
      }

      function isElementVisible(element) {
        if (!element) return false;
        if (element.type === 'hidden') return false;

        try {
          var style = window.getComputedStyle(element);
          if (
            style.display === 'none' ||
            style.visibility === 'hidden' ||
            Number(style.opacity || '1') === 0
          ) {
            return false;
          }
        } catch (_) {}

        if (typeof element.getClientRects === 'function') {
          return element.getClientRects().length > 0;
        }

        return true;
      }

      function getFieldLabel(form, element) {
        var ariaLabel = String(element.getAttribute('aria-label') || '').trim();
        if (ariaLabel) return ariaLabel;

        var labels = element.labels;
        if (labels && labels.length > 0) {
          var joined = Array.prototype.map.call(labels, function(label) {
            return String(label.textContent || '').trim();
          }).filter(Boolean).join(' ');
          if (joined) return joined;
        }

        var id = String(element.id || '').trim();
        if (id) {
          try {
            var escapedId =
              typeof CSS !== 'undefined' && typeof CSS.escape === 'function'
                ? CSS.escape(id)
                : id.replace(/"/g, '\"');
            var explicitLabel = form.querySelector('label[for="' + escapedId + '"]');
            if (explicitLabel) {
              var explicitText = String(explicitLabel.textContent || '').trim();
              if (explicitText) return explicitText;
            }
          } catch (_) {}
        }

        var parentLabel = typeof element.closest === 'function'
          ? element.closest('label')
          : null;
        if (parentLabel) {
          var parentText = String(parentLabel.textContent || '').trim();
          if (parentText) return parentText;
        }

        var parentElement = element.parentElement;
        if (parentElement) {
          var siblingLabel = parentElement.querySelector('label');
          if (siblingLabel) {
            var siblingLabelText = String(siblingLabel.textContent || '').trim();
            if (siblingLabelText) return siblingLabelText;
          }
        }

        var previousSibling = element.previousElementSibling;
        while (previousSibling) {
          if (String(previousSibling.tagName || '').toLowerCase() === 'label') {
            var previousSiblingText = String(previousSibling.textContent || '').trim();
            if (previousSiblingText) return previousSiblingText;
          }
          previousSibling = previousSibling.previousElementSibling;
        }

        var placeholder = String(element.getAttribute('placeholder') || '').trim();
        if (placeholder) return placeholder;

        return String(element.name || element.id || element.type || 'field').trim();
      }

      function addDataValue(data, name, value) {
        if (!name) return;

        if (Object.prototype.hasOwnProperty.call(data, name)) {
          if (Array.isArray(data[name])) {
            data[name].push(value);
          } else {
            data[name] = [data[name], value];
          }
          return;
        }

        data[name] = value;
      }

      function normalizeFieldType(element) {
        var tagName = String(element.tagName || '').toLowerCase();
        if (tagName === 'textarea') return 'textarea';
        if (tagName === 'select') return 'select';
        return String(element.type || tagName || 'text').toLowerCase();
      }

      function getFieldValue(element) {
        var type = normalizeFieldType(element);

        if (type === 'checkbox' || type === 'radio') {
          return element.checked ? 'true' : '';
        }

        if (type === 'file') {
          if (!element.files || !element.files.length) return '';
          return Array.prototype.map.call(element.files, function(file) {
            return file && file.name ? file.name : '';
          }).filter(Boolean).join(', ');
        }

        if (element.tagName && String(element.tagName).toLowerCase() === 'select' && element.multiple) {
          return Array.prototype.map.call(element.selectedOptions || [], function(option) {
            return option && option.value ? option.value : '';
          }).filter(Boolean).join(', ');
        }

        return String(element.value || '').trim();
      }

      function collectFormPayload(form) {
        var fields = [];
        var data = {};
        var elements = form.querySelectorAll('input, textarea, select');

        Array.prototype.forEach.call(elements, function(element, index) {
          if (!element || element.disabled) return;
          if (!isElementVisible(element)) return;

          var type = normalizeFieldType(element);
          if (
            type === 'submit' ||
            type === 'button' ||
            type === 'reset' ||
            type === 'image'
          ) {
            return;
          }

          if ((type === 'checkbox' || type === 'radio') && !element.checked) {
            return;
          }

          var value = getFieldValue(element);
          if (!value) return;

          var name = String(element.name || element.id || ('field_' + index)).trim();
          if (!name) return;

          fields.push({
            name: name,
            label: getFieldLabel(form, element),
            type: type,
            value: value
          });
          addDataValue(data, name, value);
        });

        return { fields: fields, data: data };
      }

      function getPageUrl() {
        try {
          if (window.location.href && window.location.href !== 'about:srcdoc') {
            return window.location.href;
          }
        } catch (_) {}

        try {
          return String(document.baseURI || '').trim();
        } catch (_) {
          return '';
        }
      }

      function setSubmittingState(form, isSubmitting) {
        var submitControls = form.querySelectorAll(
          'button[type="submit"], input[type="submit"], input[type="image"], [data-pixel-form-submit="true"]'
        );

        Array.prototype.forEach.call(submitControls, function(control) {
          if (!(control instanceof HTMLElement)) return;

          if (isSubmitting) {
            if (!control.hasAttribute('data-lh-original-disabled')) {
              control.setAttribute(
                'data-lh-original-disabled',
                control.hasAttribute('disabled') ? 'true' : 'false'
              );
            }
            control.setAttribute('disabled', 'disabled');
          } else if (control.getAttribute('data-lh-original-disabled') === 'false') {
            control.removeAttribute('disabled');
          }

          if (control.tagName.toLowerCase() === 'button') {
            if (isSubmitting) {
              if (!control.hasAttribute('data-lh-original-label')) {
                control.setAttribute(
                  'data-lh-original-label',
                  String(control.textContent || '')
                );
              }
              control.textContent = 'Sending...';
            } else if (control.hasAttribute('data-lh-original-label')) {
              control.textContent = control.getAttribute('data-lh-original-label') || '';
              control.removeAttribute('data-lh-original-label');
            }
          } else if (control.tagName.toLowerCase() === 'input') {
            if (isSubmitting) {
              if (!control.hasAttribute('data-lh-original-label')) {
                control.setAttribute(
                  'data-lh-original-label',
                  String(control.getAttribute('value') || '')
                );
              }
              control.setAttribute('value', 'Sending...');
            } else if (control.hasAttribute('data-lh-original-label')) {
              control.setAttribute(
                'value',
                control.getAttribute('data-lh-original-label') || ''
              );
              control.removeAttribute('data-lh-original-label');
            }
          }

          if (!isSubmitting) {
            control.removeAttribute('data-lh-original-disabled');
          }
        });
      }

      async function handleLandingHeroSubmit(payload) {
        return fetch(API_BASE_URL + '/projects/forms/submit', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(payload)
        });
      }

      async function handleWebhookSubmit(url, payload) {
        return fetch(url, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(payload)
        });
      }

      document.addEventListener('submit', async function(event) {
        var form = event.target;
        if (!(form instanceof HTMLFormElement)) return;
        if (!PROJECT_ID) return;
        if (form.__lhFormSubmitting) return;

        event.preventDefault();
        event.stopPropagation();
        event.stopImmediatePropagation();

        var payload = collectFormPayload(form);
        if (!payload.fields.length) {
          setFeedback(form, 'error', 'Please complete the form before submitting.');
          return;
        }

        form.__lhFormSubmitting = true;
        setSubmittingState(form, true);
        setFeedback(form, 'success', '');

        var mode = String(form.getAttribute('data-lh-form-mode') || 'landinghero')
          .trim()
          .toLowerCase() === 'webhook'
          ? 'webhook'
          : 'landinghero';

        var requestPayload = {
          project_id: PROJECT_ID,
          projectId: PROJECT_ID,
          submittedAt: new Date().toISOString(),
          pageUrl: getPageUrl(),
          form: {
            pixelId: String(form.getAttribute('data-pixel-id') || '').trim()
          },
          fields: payload.fields,
          data: payload.data
        };

        try {
          var response;

          if (mode === 'webhook') {
            var webhookUrl = String(form.getAttribute('data-lh-form-webhook-url') || '').trim();
            if (!webhookUrl) {
              throw new Error('Webhook URL is not configured for this form.');
            }
            response = await handleWebhookSubmit(webhookUrl, requestPayload);
          } else {
            requestPayload.recipientEmail = String(
              form.getAttribute('data-lh-form-recipient-email') || ''
            ).trim();
            response = await handleLandingHeroSubmit(requestPayload);
          }

          if (!response.ok) {
            var errorMessage = DEFAULT_ERROR_MESSAGE;
            try {
              var errorBody = await response.json();
              if (errorBody && errorBody.error) {
                errorMessage = String(errorBody.error);
              }
            } catch (_) {}
            throw new Error(errorMessage);
          }

          setFeedback(form, 'success', DEFAULT_SUCCESS_MESSAGE);
          form.reset();
        } catch (error) {
          setFeedback(
            form,
            'error',
            error && error.message ? error.message : DEFAULT_ERROR_MESSAGE
          );
        } finally {
          form.__lhFormSubmitting = false;
          setSubmittingState(form, false);
        }
      }, true);
    })();
  </script>
</body></html>)rawliteral";
