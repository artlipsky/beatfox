import { Link } from 'react-router-dom';
import ThemeToggle from './theme/ThemeToggle';

const stats = [
  { label: 'Simulation accuracy', value: '+/- 1.2 dB' },
  { label: '3D scenes analysed', value: '5k+' },
  { label: 'Material presets', value: '320' }
];

const features = [
  {
    icon: '🌀',
    title: 'Volumetric Wave Solver',
    description: 'Capture complex reflections, refractions, and diffraction across irregular architectural geometries.'
  },
  {
    icon: '🎛️',
    title: 'Material Intelligence',
    description: 'Blend custom absorption curves and scattering profiles to audition finishes in context.'
  },
  {
    icon: '🛰️',
    title: 'Spatial Listening Paths',
    description: 'Trace multiple listener positions to compare intelligibility, localization, and RT60 in real time.'
  },
  {
    icon: '⚡',
    title: 'GPU-Accelerated Renderer',
    description: 'Streamline iterations with lightning-fast previews and export-ready acoustic heat maps.'
  }
];

const workflow = [
  {
    title: 'Model your space',
    description: 'Import CAD geometry or sketch volumes in minutes with our guided scene builder.'
  },
  {
    title: 'Place sources & listeners',
    description: 'Assign directivity, frequency response, and motion paths to simulate realistic performances.'
  },
  {
    title: 'Run multi-band analysis',
    description: 'Compute wave propagation, early reflections, and reverberant fields across custom bands.'
  },
  {
    title: 'Share interactive insights',
    description: 'Export immersive viewers, reports, and mix-ready impulse responses with a click.'
  }
];

const audiences = [
  'Architectural acoustics',
  'Performance venues',
  'Post-production suites',
  'Immersive installations',
  'Urban soundscapes',
  'Advanced education'
];

const LandingPage = () => {
  return (
    <div className="relative min-h-screen overflow-hidden bg-slate-50 text-slate-900 transition-colors dark:bg-slate-950 dark:text-slate-100">
      <div aria-hidden className="pointer-events-none absolute inset-0 overflow-hidden">
        <div className="absolute left-[8%] top-[-6rem] h-72 w-72 rounded-full bg-blue-500/20 blur-3xl dark:bg-blue-500/10" />
        <div className="absolute right-[-8rem] top-20 h-96 w-96 rounded-full bg-purple-500/20 blur-3xl dark:bg-purple-500/10" />
        <div className="absolute bottom-[-10rem] left-1/2 h-[28rem] w-[28rem] -translate-x-1/2 rounded-full bg-sky-400/10 blur-3xl dark:bg-sky-400/5" />
      </div>

      <div className="relative">
        <header className="sticky top-0 z-20 border-b border-slate-200/60 bg-white/70 backdrop-blur-xl transition dark:border-slate-800/80 dark:bg-slate-950/60">
          <div className="mx-auto flex max-w-6xl items-center justify-between px-4 py-4 lg:px-8">
            <Link to="/" className="text-lg font-semibold tracking-wide">
              <span className="bg-gradient-to-r from-blue-600 via-purple-500 to-sky-500 bg-clip-text text-transparent">BeatFox</span>
            </Link>
            <nav className="hidden items-center gap-8 text-sm font-medium text-slate-600 dark:text-slate-300 md:flex">
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="#features">
                Features
              </a>
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="#workflow">
                Workflow
              </a>
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="#audience">
                Teams
              </a>
            </nav>
            <Link
              to="/demo"
              className="hidden rounded-full bg-slate-900 px-4 py-2 text-sm font-semibold text-white transition hover:bg-slate-700 dark:bg-white dark:text-slate-900 dark:hover:bg-slate-200 md:inline-flex"
            >
              Launch demo
            </Link>
          </div>
        </header>

        <main>
          <section className="mx-auto flex max-w-6xl flex-col items-start gap-12 px-4 pb-20 pt-16 lg:flex-row lg:items-center lg:gap-20 lg:px-8">
            <div className="flex-1 space-y-10">
              <div className="inline-flex items-center gap-2 rounded-full border border-slate-200 bg-white/80 px-3 py-1 text-xs font-semibold uppercase tracking-[0.2em] text-slate-600 shadow-sm backdrop-blur dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-300">
                Spatial acoustics intelligence
              </div>
              <div className="space-y-6">
                <h1 className="text-balance text-4xl font-semibold leading-tight text-slate-900 sm:text-5xl lg:text-6xl dark:text-white">
                  Sculpt immersive soundscapes with scientific precision
                </h1>
                <p className="max-w-xl text-lg text-slate-600 dark:text-slate-300">
                  BeatFox simulates how sound breathes through your architecture-visualize energy, trace reflections, and tune clarity before the first wall is built.
                </p>
              </div>
              <div className="flex w-full flex-col gap-3 sm:flex-row sm:items-center">
                <Link
                  to="/demo"
                  className="inline-flex items-center justify-center gap-2 rounded-full bg-slate-900 px-7 py-3 text-base font-semibold text-white shadow-lg transition hover:-translate-y-0.5 hover:shadow-xl dark:bg-white dark:text-slate-900"
                >
                  Launch interactive demo
                  <span aria-hidden>→</span>
                </Link>
                <a
                  href="#features"
                  className="inline-flex items-center justify-center gap-2 rounded-full border border-slate-300/80 bg-white/70 px-7 py-3 text-base font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-300 dark:hover:border-slate-500 dark:hover:text-white"
                >
                  Explore capabilities
                </a>
              </div>
              <div className="grid w-full grid-cols-1 gap-4 sm:grid-cols-3">
                {stats.map((item) => (
                  <div
                    key={item.label}
                    className="rounded-2xl border border-slate-200/70 bg-white/80 p-5 text-left shadow-sm backdrop-blur dark:border-slate-800 dark:bg-slate-900/80"
                  >
                    <p className="text-sm font-medium text-slate-600 dark:text-slate-400">{item.label}</p>
                    <p className="mt-2 text-2xl font-semibold text-slate-900 dark:text-white">{item.value}</p>
                  </div>
                ))}
              </div>
            </div>

            <div className="relative flex-1">
              <div className="absolute left-1/2 top-10 hidden h-60 w-60 -translate-x-1/2 rounded-full bg-blue-500/20 blur-3xl dark:bg-blue-500/10 lg:block" />
              <div className="relative overflow-hidden rounded-3xl border border-slate-200/80 bg-white/90 shadow-2xl shadow-blue-500/10 backdrop-blur dark:border-slate-800/80 dark:bg-slate-900/80">
                <div className="flex items-center justify-between border-b border-slate-200/80 px-6 py-4 text-sm font-semibold text-slate-600 dark:border-slate-800 dark:text-slate-300">
                  <span>Acoustic field preview</span>
                  <span className="inline-flex items-center gap-1 rounded-full bg-blue-100 px-3 py-1 text-xs font-semibold text-blue-600 dark:bg-blue-500/20 dark:text-blue-200">
                    Live
                  </span>
                </div>
                <div className="relative space-y-6 px-6 pb-8 pt-6">
                  <div className="flex gap-4">
                    <div className="flex-1 rounded-2xl bg-gradient-to-br from-blue-500/20 via-purple-500/10 to-sky-500/10 p-4 dark:from-blue-500/10 dark:via-purple-500/10 dark:to-sky-500/5">
                      <div className="h-40 rounded-xl border border-dashed border-blue-400/40 bg-gradient-to-br from-slate-50/60 via-blue-100/40 to-purple-100/30 p-4 dark:from-slate-900/40 dark:via-blue-900/30 dark:to-purple-900/20">
                        <div className="h-full w-full rounded-lg bg-[radial-gradient(circle_at_top,_#60a5fa_0%,_transparent_55%)] dark:bg-[radial-gradient(circle_at_top,_rgba(96,165,250,0.4)_0%,_transparent_55%)]" />
                      </div>
                    </div>
                    <div className="flex w-28 flex-col gap-3">
                      {[68, 54, 47].map((value) => (
                        <div
                          key={value}
                          className="flex-1 rounded-2xl border border-slate-200/80 bg-white/80 p-3 text-sm dark:border-slate-800 dark:bg-slate-900/60"
                        >
                          <p className="text-xs text-slate-500 dark:text-slate-400">RT60</p>
                          <p className="text-lg font-semibold text-slate-900 dark:text-white">{value} ms</p>
                        </div>
                      ))}
                    </div>
                  </div>
                  <div className="rounded-2xl border border-slate-200/80 bg-slate-900/90 px-5 py-4 text-sm text-white shadow-inner dark:border-slate-700 dark:bg-slate-100 dark:text-slate-900">
                    <div className="flex items-center justify-between">
                      <span className="font-semibold">Clarity @ Listener B</span>
                      <span className="rounded-full bg-white/20 px-3 py-1 text-xs font-medium tracking-wide text-white dark:bg-slate-900/80 dark:text-slate-100">
                        +3.7 dB
                      </span>
                    </div>
                    <p className="mt-2 text-xs opacity-80">
                      Optimized with diffusive ceiling panels and transparent acoustic glazing.
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </section>

          <section id="features" className="relative border-y border-slate-200/60 bg-white/70 py-20 backdrop-blur dark:border-slate-800/80 dark:bg-slate-950/60">
            <div className="mx-auto max-w-6xl px-4 lg:px-8">
              <div className="text-center">
                <p className="text-sm font-semibold uppercase tracking-[0.35em] text-slate-500 dark:text-slate-400">Capabilities</p>
                <h2 className="mt-3 text-3xl font-semibold text-slate-900 sm:text-4xl dark:text-white">
                  Designed for the sound architects of tomorrow
                </h2>
                <p className="mx-auto mt-4 max-w-2xl text-base text-slate-600 dark:text-slate-300">
                  A toolkit that unites physical accuracy with expressive storytelling, so your spaces perform exactly as imagined.
                </p>
              </div>
              <div className="mt-14 grid gap-6 sm:grid-cols-2">
                {features.map((feature) => (
                  <div
                    key={feature.title}
                    className="group relative overflow-hidden rounded-3xl border border-slate-200/70 bg-white/90 p-6 shadow-sm transition hover:-translate-y-1 hover:shadow-xl dark:border-slate-800 dark:bg-slate-900/70"
                  >
                    <div className="absolute -right-10 top-10 h-28 w-28 rounded-full bg-blue-500/10 blur-2xl transition group-hover:scale-110 dark:bg-blue-500/5" />
                    <div className="relative">
                      <span className="text-3xl">{feature.icon}</span>
                      <h3 className="mt-4 text-xl font-semibold text-slate-900 dark:text-white">{feature.title}</h3>
                      <p className="mt-3 text-sm text-slate-600 dark:text-slate-300">{feature.description}</p>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </section>

          <section id="workflow" className="mx-auto max-w-6xl px-4 py-20 lg:px-8">
            <div className="grid gap-16 lg:grid-cols-[0.65fr_0.35fr] lg:items-center">
              <div>
                <p className="text-sm font-semibold uppercase tracking-[0.35em] text-slate-500 dark:text-slate-400">Workflow</p>
                <h2 className="mt-3 text-3xl font-semibold text-slate-900 sm:text-4xl dark:text-white">
                  A seamless path from geometry to experience
                </h2>
                <p className="mt-4 max-w-2xl text-base text-slate-600 dark:text-slate-300">
                  BeatFox streamlines every decision with contextual insights, guiding you toward sonic clarity without breaking your flow.
                </p>
                <div className="mt-10 space-y-10">
                  {workflow.map((step, index) => (
                    <div key={step.title} className="relative pl-12">
                      <div className="absolute left-0 top-1 flex h-8 w-8 items-center justify-center rounded-full border border-blue-500/30 bg-blue-500/10 text-sm font-semibold text-blue-600 dark:border-blue-400/30 dark:bg-blue-500/20 dark:text-blue-200">
                        {index + 1}
                      </div>
                      <h3 className="text-lg font-semibold text-slate-900 dark:text-white">{step.title}</h3>
                      <p className="mt-2 text-sm text-slate-600 dark:text-slate-300">{step.description}</p>
                    </div>
                  ))}
                </div>
              </div>
              <div className="space-y-6 rounded-3xl border border-slate-200/70 bg-white/80 p-8 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
                <p className="text-sm font-semibold uppercase tracking-[0.3em] text-slate-500 dark:text-slate-400">Why it works</p>
                <h3 className="text-2xl font-semibold text-slate-900 dark:text-white">Design decisions, grounded in data</h3>
                <p className="text-sm text-slate-600 dark:text-slate-300">
                  Layer wave-based solvers with perceptual metrics, so you can articulate choices to clients and collaborators with confidence.
                </p>
                <ul className="space-y-3 text-sm text-slate-600 dark:text-slate-300">
                  <li className="flex items-start gap-3">
                    <span className="mt-0.5 text-lg">•</span>
                    Multi-band RT, C80, STI, and custom KPIs
                  </li>
                  <li className="flex items-start gap-3">
                    <span className="mt-0.5 text-lg">•</span>
                    Version snapshots with diff overlays
                  </li>
                  <li className="flex items-start gap-3">
                    <span className="mt-0.5 text-lg">•</span>
                    Team workspaces with design annotations
                  </li>
                </ul>
                <Link
                  to="/demo"
                  className="inline-flex w-full items-center justify-center gap-2 rounded-full border border-slate-300/80 px-6 py-3 text-sm font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:text-slate-200 dark:hover:border-slate-500"
                >
                  Preview the demo
                  <span aria-hidden>→</span>
                </Link>
              </div>
            </div>
          </section>

          <section id="audience" className="border-y border-slate-200/60 bg-white/70 py-16 backdrop-blur dark:border-slate-800/80 dark:bg-slate-950/60">
            <div className="mx-auto max-w-6xl px-4 text-center lg:px-8">
              <p className="text-sm font-semibold uppercase tracking-[0.35em] text-slate-500 dark:text-slate-400">Trusted across disciplines</p>
              <h2 className="mt-3 text-3xl font-semibold text-slate-900 sm:text-4xl dark:text-white">Crafted for teams shaping the future of sound</h2>
              <p className="mx-auto mt-4 max-w-2xl text-base text-slate-600 dark:text-slate-300">
                From flagship performance halls to intimate immersive rooms, BeatFox adapts to the scale and nuance of every brief.
              </p>
              <div className="mt-10 flex flex-wrap justify-center gap-3">
                {audiences.map((audience) => (
                  <span
                    key={audience}
                    className="rounded-full border border-slate-200/70 bg-white/80 px-4 py-2 text-sm font-medium text-slate-600 shadow-sm transition dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-300"
                  >
                    {audience}
                  </span>
                ))}
              </div>
            </div>
          </section>

          <section className="relative mx-auto max-w-4xl px-4 py-20 text-center lg:px-8">
            <div className="absolute inset-0 -z-10 rounded-3xl bg-gradient-to-br from-slate-900 via-blue-900 to-purple-900 opacity-90 blur-xl dark:opacity-80" />
            <div className="relative overflow-hidden rounded-3xl border border-slate-200/70 bg-white/90 p-10 shadow-2xl dark:border-slate-800 dark:bg-slate-900/80">
              <h2 className="text-3xl font-semibold text-slate-900 dark:text-white">Bring your acoustic vision to life</h2>
              <p className="mx-auto mt-3 max-w-xl text-base text-slate-600 dark:text-slate-300">
                Start exploring complex wave behaviour with intuitive controls, interactive visualizations, and shareable insights.
              </p>
              <div className="mt-8 flex flex-col items-center justify-center gap-3 sm:flex-row">
                <Link
                  to="/demo"
                  className="inline-flex min-w-[200px] items-center justify-center rounded-full bg-slate-900 px-8 py-3 text-base font-semibold text-white shadow-lg transition hover:-translate-y-0.5 hover:shadow-xl dark:bg-white dark:text-slate-900"
                >
                  Try the demo
                </Link>
                <a
                  href="mailto:hello@beatfox.io"
                  className="inline-flex min-w-[200px] items-center justify-center rounded-full border border-slate-300/80 bg-white/80 px-8 py-3 text-base font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-200"
                >
                  Talk with our team
                </a>
              </div>
            </div>
          </section>
        </main>

        <footer className="border-t border-slate-200/60 bg-white/70 py-8 text-sm text-slate-500 backdrop-blur dark:border-slate-800/80 dark:bg-slate-950/60 dark:text-slate-400">
          <div className="mx-auto flex max-w-6xl flex-col items-center justify-between gap-4 px-4 text-center sm:flex-row sm:text-left lg:px-8">
            <span>© {new Date().getFullYear()} BeatFox Acoustic Intelligence</span>
            <div className="flex flex-wrap items-center justify-center gap-4">
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="#features">
                Product
              </a>
              <Link className="transition hover:text-slate-900 dark:hover:text-white" to="/demo">
                Demo
              </Link>
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="mailto:hello@beatfox.io">
                Contact
              </a>
              <span className="hidden sm:inline text-slate-300 dark:text-slate-600">·</span>
              <ThemeToggle />
            </div>
          </div>
        </footer>
      </div>
    </div>
  );
};

export default LandingPage;
