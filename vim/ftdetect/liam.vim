au BufRead,BufNewFile *.liam set filetype=liam
if has("nvim")
  let s:started = 0
  function! s:Start()
    if s:started
      return
    end
    let s:started = 1
  endfunction
  au FileType liam call s:Start()
end
