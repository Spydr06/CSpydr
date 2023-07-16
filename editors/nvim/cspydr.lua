-- CSpydr neovim plugin

local function setup()
    vim.cmd([[
        autocmd BufRead,BufNewFile *.csp set filetype=cspydr
        autocmd Syntax cspydr runtime! syntax/cspydr.vim
    ]])
end

return {
    setup = setup,
}