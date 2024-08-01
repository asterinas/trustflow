# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "TrustFlow"
copyright = "2023 Ant Group Co., Ltd."
author = "SecretFlow authors"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["myst_nb"]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for  sphinx-intl -------------------------------------------------
locale_dirs = ["locale/"]  # path is example but recommended.
gettext_compact = False  # optional.

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"
html_static_path = []
html_css_files = []
html_js_files = []
html_theme_options = {
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/asterinas/trustflow",
            "icon": "fab fa-github-square",
            "type": "fontawesome",
        },
    ],
    "logo": {
        "text": "TrustFlow",
    },
    "show_nav_level": 4,
}

# Options for MyST-Parser
# https://myst-parser.readthedocs.io/

# Enable all MyST features
myst_gfm_only = False
# Enable anchors for heading level h1 through h6
myst_heading_anchors = 6

# Enable TODO
todo_include_todos = True

myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
    "attrs_inline",
    "attrs_block",
]

# Options for MyST-NB
# https://myst-nb.readthedocs.io/

nb_execution_mode = "off"

autodoc_default_options = {
    "members": True,
    "member-order": "bysource",
    "special-members": "__init__",
    "undoc-members": False,
    "show-inheritance": False,
}
