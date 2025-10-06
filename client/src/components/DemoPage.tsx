import { Link } from 'react-router-dom';
import {
  AdjustmentsHorizontalIcon,
  ArrowLeftIcon,
  CpuChipIcon,
  EnvelopeIcon,
  SpeakerWaveIcon,
  SparklesIcon,
  GlobeAltIcon,
} from '@heroicons/react/24/outline';
import { PlayCircleIcon } from '@heroicons/react/24/solid';
import ThemeToggle from './theme/ThemeToggle';
import { SoundWaveDemo } from './SoundWaveVisualization';

const DemoPage = () => {
  return (
    <div className="relative min-h-screen overflow-hidden bg-slate-50 text-slate-900 transition-colors dark:bg-slate-950 dark:text-slate-100">
      <div aria-hidden className="pointer-events-none absolute inset-0 overflow-hidden">
        <div className="absolute left-[-4rem] top-28 h-64 w-64 rounded-full bg-blue-400/20 blur-3xl dark:bg-blue-500/10" />
        <div className="absolute right-[-6rem] top-[-6rem] h-96 w-96 rounded-full bg-purple-500/15 blur-3xl dark:bg-purple-500/10" />
        <div className="absolute bottom-[-8rem] left-1/2 h-[26rem] w-[26rem] -translate-x-1/2 rounded-full bg-slate-400/10 blur-3xl dark:bg-slate-500/5" />
      </div>

      <div className="relative">
        <header className="sticky top-0 z-20 border-b border-slate-200/60 bg-white/70 backdrop-blur-xl transition dark:border-slate-800/80 dark:bg-slate-950/60">
          <div className="mx-auto flex max-w-6xl items-center justify-between px-4 py-4 lg:px-8">
            <Link to="/" className="text-lg font-semibold tracking-wide">
              <span className="bg-gradient-to-r from-blue-600 via-purple-500 to-sky-500 bg-clip-text text-transparent">BeatFox</span>
            </Link>
            <div className="flex items-center gap-3">
              <ThemeToggle />
              <Link
                to="/"
                className="hidden items-center gap-2 rounded-full border border-slate-300/80 px-4 py-2 text-sm font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:text-slate-200 dark:hover:border-slate-500 md:inline-flex"
              >
                <ArrowLeftIcon aria-hidden className="h-4 w-4" />
                <span>Back to landing</span>
              </Link>
            </div>
          </div>
        </header>

        <main className="mx-auto max-w-7xl px-4 pb-20 pt-16 lg:px-8">
          {/* Hero Section */}
          <div className="mb-12 text-center">
            <p className="mb-4 inline-flex items-center gap-2 rounded-full border border-slate-200/80 bg-white/90 px-3 py-1 text-xs font-semibold uppercase tracking-[0.3em] text-slate-600 shadow-sm backdrop-blur dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-300">
              <SparklesIcon aria-hidden className="h-4 w-4 text-blue-500 dark:text-blue-300" />
              Studio-grade acoustic lab
            </p>
            <h1 className="text-balance text-4xl font-semibold text-slate-900 sm:text-5xl dark:text-white">
              Shape Mix Rooms and Venues in Real Time
            </h1>
            <p className="mx-auto mt-4 max-w-3xl text-base text-slate-600 dark:text-slate-300">
              Balance low-end build-up, vocal clarity, and isolation strategies through an interactive 3D simulation.
              Dial in source placement, tune absorbers, and compare construction assemblies before the first panel goes up.
            </p>

            <div className="mx-auto mt-10 grid max-w-4xl gap-4 text-left sm:grid-cols-3">
              <div className="rounded-2xl border border-slate-200/80 bg-white/90 p-4 shadow-sm backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
                <div className="flex items-center gap-3">
                  <SpeakerWaveIcon aria-hidden className="h-6 w-6 text-blue-500 dark:text-blue-300" />
                  <div>
                    <p className="text-xs font-semibold uppercase tracking-wide text-slate-500 dark:text-slate-400">
                      Mix-critical bandwidth
                    </p>
                    <p className="text-base font-semibold text-slate-900 dark:text-white">Sub bass to air band (20 Hz – 20 kHz)</p>
                  </div>
                </div>
              </div>
              <div className="rounded-2xl border border-slate-200/80 bg-white/90 p-4 shadow-sm backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
                <div className="flex items-center gap-3">
                  <CpuChipIcon aria-hidden className="h-6 w-6 text-purple-500 dark:text-purple-300" />
                  <div>
                    <p className="text-xs font-semibold uppercase tracking-wide text-slate-500 dark:text-slate-400">
                      Real-time response
                    </p>
                    <p className="text-base font-semibold text-slate-900 dark:text-white">Sub-16 ms update latency</p>
                  </div>
                </div>
              </div>
              <div className="rounded-2xl border border-slate-200/80 bg-white/90 p-4 shadow-sm backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
                <div className="flex items-center gap-3">
                  <GlobeAltIcon aria-hidden className="h-6 w-6 text-sky-500 dark:text-sky-300" />
                  <div>
                    <p className="text-xs font-semibold uppercase tracking-wide text-slate-500 dark:text-slate-400">
                      Envelope coverage
                    </p>
                    <p className="text-base font-semibold text-slate-900 dark:text-white">Room-scale 125 × 125 grid</p>
                  </div>
                </div>
              </div>
            </div>
          </div>

          {/* Interactive Visualization */}
          <section className="mb-14 space-y-6">
            <div className="flex flex-col gap-4 rounded-2xl border border-slate-200/70 bg-white/90 p-6 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70 dark:shadow-none md:flex-row md:items-center md:justify-between">
              <div className="flex items-start gap-4">
                <span className="flex h-12 w-12 items-center justify-center rounded-full bg-gradient-to-br from-blue-500 via-purple-500 to-sky-500 text-white shadow-lg">
                  <PlayCircleIcon aria-hidden className="h-6 w-6" />
                </span>
                <div>
                  <h2 className="text-left text-xl font-semibold text-slate-900 dark:text-white">
                    Live Wavefield Explorer
                  </h2>
                  <p className="mt-1 max-w-xl text-left text-sm text-slate-600 dark:text-slate-300">
                    Drag to orbit, scroll to zoom, and use the panels to sculpt the wave sources. The visualization reacts instantly to your inputs.
                  </p>
                </div>
              </div>
              <div className="flex items-center gap-3 rounded-full border border-slate-200/80 bg-white/90 px-4 py-2 text-xs font-medium uppercase tracking-[0.25em] text-slate-500 shadow-sm dark:border-slate-800 dark:bg-slate-950/70 dark:text-slate-400">
                <AdjustmentsHorizontalIcon aria-hidden className="h-4 w-4" />
                Fine control enabled
              </div>
            </div>
            <SoundWaveDemo />
          </section>

          {/* Physics Information */}
          <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
            <div className="rounded-2xl border border-slate-200/70 bg-white/80 p-6 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
              <div className="mb-4 flex h-11 w-11 items-center justify-center rounded-xl bg-gradient-to-br from-blue-500/90 to-sky-500/80 text-white shadow-md">
                <SpeakerWaveIcon aria-hidden className="h-5 w-5" />
              </div>
              <h3 className="mb-3 text-lg font-semibold text-slate-900 dark:text-white">
                Wave Equation
              </h3>
              <p className="mb-2 font-mono text-sm text-slate-600 dark:text-slate-300">
                ∂²p/∂t² = c²∇²p
              </p>
              <p className="text-sm text-slate-600 dark:text-slate-300">
                Predict pressure build-up behind consoles, iso booths, and stage shells. <em>p</em> is the sound pressure field, <em>c</em> the speed of sound (343 m/s in air), and ∇² the Laplacian capturing room geometry.
              </p>
            </div>

            <div className="rounded-2xl border border-slate-200/70 bg-white/80 p-6 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
              <div className="mb-4 flex h-11 w-11 items-center justify-center rounded-xl bg-gradient-to-br from-purple-500/90 to-fuchsia-500/80 text-white shadow-md">
                <AdjustmentsHorizontalIcon aria-hidden className="h-5 w-5" />
              </div>
              <h3 className="mb-3 text-lg font-semibold text-slate-900 dark:text-white">
                Superposition
              </h3>
              <p className="mb-2 text-sm text-slate-600 dark:text-slate-300">
                Line arrays, fills, and subs blend through superposition—phase alignment locks in punch while misalignment smears the imaging.
                Dial offsets to hear constructive versus destructive interference instantly.
              </p>
              <p className="mt-2 text-xs text-slate-500 dark:text-slate-400">
                Try offsetting polarity to expose cancellations before you rewire the rig on-site.
              </p>
            </div>

            <div className="rounded-2xl border border-slate-200/70 bg-white/80 p-6 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
              <div className="mb-4 flex h-11 w-11 items-center justify-center rounded-xl bg-gradient-to-br from-slate-500/90 to-slate-700/80 text-white shadow-md dark:from-slate-400/90 dark:to-slate-600/80">
                <GlobeAltIcon aria-hidden className="h-5 w-5" />
              </div>
              <h3 className="mb-3 text-lg font-semibold text-slate-900 dark:text-white">
                Spherical Spreading
              </h3>
              <p className="mb-2 text-sm text-slate-600 dark:text-slate-300">
                Sound intensity falls off with distance (inverse square law), so isolation assemblies must double down on absorption and mass in exposed zones.
                The simulation models amplitude decay as 1/r to highlight where additional treatment or baffling is required.
              </p>
              <p className="mt-2 font-mono text-xs text-slate-500 dark:text-slate-400">
                I ∝ 1/r²
              </p>
            </div>
          </div>

          <section className="mt-12 rounded-3xl border border-slate-200/70 bg-white/90 p-8 shadow-xl backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
            <div className="flex flex-col gap-6 md:flex-row md:items-start md:justify-between">
              <div className="max-w-xl text-left">
                <h3 className="text-2xl font-semibold text-slate-900 dark:text-white">
                  Tune studios and assemblies faster
                </h3>
                <p className="mt-2 text-sm text-slate-600 dark:text-slate-300">
                  Built for mix engineers, venue consultants, and builders validating isolation targets before construction crews roll in.
                </p>
              </div>
              <div className="grid gap-4 text-left">
                <div className="flex items-start gap-3 rounded-2xl border border-slate-200/70 bg-white/90 p-4 shadow-sm dark:border-slate-800 dark:bg-slate-950/50">
                  <PlayCircleIcon aria-hidden className="mt-0.5 h-6 w-6 text-blue-500" />
                  <div>
                    <p className="font-semibold text-slate-900 dark:text-white">Position monitors and subs</p>
                    <p className="text-sm text-slate-600 dark:text-slate-300">
                      Toggle sources to balance direct and reflected energy across the listening couch, FOH desk, or live room floor.
                    </p>
                  </div>
                </div>
                <div className="flex items-start gap-3 rounded-2xl border border-slate-200/70 bg-white/90 p-4 shadow-sm dark:border-slate-800 dark:bg-slate-950/50">
                  <AdjustmentsHorizontalIcon aria-hidden className="mt-0.5 h-6 w-6 text-purple-500" />
                  <div>
                    <p className="font-semibold text-slate-900 dark:text-white">Sweep for mix translation</p>
                    <p className="text-sm text-slate-600 dark:text-slate-300">
                      Track down modal spikes and flutter echoes as you glide through octave bands that matter for music and speech intelligibility.
                    </p>
                  </div>
                </div>
                <div className="flex items-start gap-3 rounded-2xl border border-slate-200/70 bg-white/90 p-4 shadow-sm dark:border-slate-800 dark:bg-slate-950/50">
                  <CpuChipIcon aria-hidden className="mt-0.5 h-6 w-6 text-emerald-500" />
                  <div>
                    <p className="font-semibold text-slate-900 dark:text-white">Prototype isolation build-ups</p>
                    <p className="text-sm text-slate-600 dark:text-slate-300">
                      Compare staggered studs, resilient channels, or floating floors and share the results with architects and contractors.
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </section>

          {/* Call to Action */}
          <section className="mt-16 rounded-3xl border border-slate-200/70 bg-white/80 p-10 text-center shadow-xl backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
            <h2 className="text-3xl font-semibold text-slate-900 dark:text-white">
              Ready for Production-Ready Acoustic Simulation?
            </h2>
            <p className="mx-auto mt-3 max-w-2xl text-base text-slate-600 dark:text-slate-300">
              The demo showcases core propagation physics. BeatFox expands this into full mix-room tuning, venue coverage studies,
              and wall assembly isolation forecasting—all in a workflow shared by engineers, consultants, and builders.
            </p>
            <div className="mt-8 flex flex-col items-center justify-center gap-3 sm:flex-row">
              <a
                href="mailto:hello@beatfox.io"
                className="inline-flex items-center justify-center gap-2 rounded-full bg-slate-900 px-7 py-3 text-base font-semibold text-white shadow-lg transition hover:-translate-y-0.5 hover:shadow-xl dark:bg-white dark:text-slate-900"
              >
                <EnvelopeIcon aria-hidden className="h-5 w-5" />
                <span>Join the beta list</span>
              </a>
              <Link
                to="/"
                className="inline-flex items-center justify-center gap-2 rounded-full border border-slate-300/80 bg-white/70 px-7 py-3 text-base font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-200 dark:hover:border-slate-500"
              >
                <GlobeAltIcon aria-hidden className="h-5 w-5" />
                <span>Explore the product</span>
              </Link>
            </div>
          </section>
        </main>

        <footer className="border-t border-slate-200/60 bg-white/70 py-8 text-sm text-slate-500 backdrop-blur dark:border-slate-800/80 dark:bg-slate-950/60 dark:text-slate-400">
          <div className="mx-auto flex max-w-6xl flex-col items-center justify-between gap-4 px-4 text-center sm:flex-row sm:text-left lg:px-8">
            <span>© {new Date().getFullYear()} BeatFox Acoustic Intelligence</span>
            <div className="flex flex-wrap justify-center gap-4">
              <Link className="transition hover:text-slate-900 dark:hover:text-white" to="/">
                Landing
              </Link>
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="mailto:hello@beatfox.io">
                Contact
              </a>
            </div>
          </div>
        </footer>
      </div>
    </div>
  );
};

export default DemoPage;
