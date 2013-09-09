%% pset.cls (A class file for problem sets)
%%
%% version 1
%% 8 September 2013

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{pset}
\LoadClass[12pt]{article}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Package Imports %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\RequirePackage{jdh}
\RequirePackage{hyperref}
\RequirePackage{fourier}
\RequirePackage{DejaVuSansMono}
\RequirePackage[letterpaper,margin=1in]{geometry}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Page Layout %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\setlength\parindent{1em}
\addtolength\parskip{0.1cm}
\setlength\fboxrule{.5mm}\setlength{\fboxsep}{1.2mm}
\newlength\boxlength\setlength\boxlength\textwidth
\addtolength\boxlength{-4mm}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Assignment Header %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\newcommand\@psnum{}
\newcommand\@psname{}
\newcommand\psname[1]{\renewcommand\@psname{#1}}
\newcommand\psnum[1]{\renewcommand\@psnum{#1}}

\renewcommand\@date{}
\renewcommand\date[1]{\renewcommand{\@date}{#1}}

\renewcommand\maketitle{
  \vspace*{-5em}
  \begin{center}\framebox{\parbox{\boxlength}{\bf
    Josh Hagins\hfill jdh298\\
    Introduction to Compilers\hfill
    Problem Set \@psnum \ifx\@psname\empty\else: \@psname\fi\\ 
    CS 4120, Fall 2013\hfill \@date
  }}
  \end{center}
  \vspace{1em}
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%% Problems and solutions %%%%%%%%%%%%%%%%%%%%%%%%%%%%

\newcounter{problem}
\newcounter{subproblem}[problem]
\newcounter{subsubproblem}[subproblem]

\newcommand\bracket[1]{\ifx#1\empty\else[#1]\fi}

\newcommand\problem[1]{
  \stepcounter{problem}
  \section*{Problem \theproblem\ifx#1\empty\else: #1\fi}
}

\newcommand\subproblem[1]{
  \stepcounter{subproblem}
  \subsection*{Part (\alph{subproblem})\ifx#1\empty\else: #1\fi}
}

\newcommand\subsubproblem[1]{
  \stepcounter{subsubproblem}
  \subsection*{(\roman{subproblem})\ifx#1\empty\else: #1\fi}
}

%
% vim: syntax=tex
%

%%
%% End of file 'pset.cls'
